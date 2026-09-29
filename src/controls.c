#include "controls.h"

#include <pthread.h>
#include <stdbool.h>
#include <syslog.h>

#include "linuxdoom/doomdef.h"

#define INPUT_QUEUE_CAPACITY 256
#define INPUT_PROTOCOL_VERSION 1
#define INPUT_MESSAGE_KEY 1
#define INPUT_MESSAGE_RESET 5

struct input_event {
  uint16_t key;
  bool down;
};

/* The WebSocket thread produces input and the game thread consumes it */
static pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct input_event input_queue[INPUT_QUEUE_CAPACITY];
static size_t input_head;
static size_t input_count;

/* Key states belong to the game thread */
static bool pressed_keys[ACAP_KEY_COUNT];
static unsigned int reset_key;

static int
translate_key(unsigned int key)
{
  /* Letter keys */
  if (key >= ACAP_KEY_A && key <= ACAP_KEY_Z) {
    return 'a' + key - ACAP_KEY_A;
  }

  /* Number keys */
  if (key >= ACAP_KEY_DIGIT_0 && key <= ACAP_KEY_DIGIT_9) {
    return '0' + key - ACAP_KEY_DIGIT_0;
  }

  switch (key) {
  /* Arrow keys */
  case ACAP_KEY_ARROW_UP:
    return KEY_UPARROW;
  case ACAP_KEY_ARROW_DOWN:
    return KEY_DOWNARROW;
  case ACAP_KEY_ARROW_LEFT:
    return KEY_LEFTARROW;
  case ACAP_KEY_ARROW_RIGHT:
    return KEY_RIGHTARROW;
  /* Control keys */
  case ACAP_KEY_ENTER:
    return KEY_ENTER;
  case ACAP_KEY_ESCAPE:
    return KEY_ESCAPE;
  case ACAP_KEY_TAB:
    return KEY_TAB;
  case ACAP_KEY_SPACE:
    return KEY_SPACE;
  case ACAP_KEY_BACKSPACE:
    return KEY_BACKSPACE;
  case ACAP_KEY_SHIFT_LEFT:
  case ACAP_KEY_SHIFT_RIGHT:
    return KEY_RSHIFT;
  case ACAP_KEY_CTRL_LEFT:
  case ACAP_KEY_CTRL_RIGHT:
    return KEY_RCTRL;
  case ACAP_KEY_ALT_LEFT:
  case ACAP_KEY_ALT_RIGHT:
    return KEY_RALT;
  /* Character keys */
  case ACAP_KEY_COMMA:
    return ',';
  case ACAP_KEY_PERIOD:
    return '.';
  default:
    return 0;
  }
}

static void
enqueue_input(struct input_event input)
{
  pthread_mutex_lock(&input_mutex);

  /* A reset or overflow discards stale input before releasing held keys */
  if (input.key == ACAP_KEY_UNKNOWN || input_count == INPUT_QUEUE_CAPACITY) {
    if (input_count == INPUT_QUEUE_CAPACITY) {
      syslog(LOG_WARNING, "Input queue full, resetting controls");
    }
    input_head = 0;
    input_count = 0;
    input = (struct input_event) { 0 };
  }

  size_t index = (input_head + input_count) % INPUT_QUEUE_CAPACITY;
  input_queue[index] = input;
  input_count++;

  pthread_mutex_unlock(&input_mutex);
}

void
controls_reset(void)
{
  enqueue_input((struct input_event) { 0 });
}

int
controls_receive(const uint8_t *data, size_t len)
{
  if (!data || len < 2 || data[0] != INPUT_PROTOCOL_VERSION) {
    return false;
  }

  if (data[1] == INPUT_MESSAGE_RESET && len == 2) {
    controls_reset();
    return true;
  }

  if (data[1] != INPUT_MESSAGE_KEY || len != 5 || data[4] > 1) {
    return false;
  }

  uint16_t key = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
  if (key == ACAP_KEY_UNKNOWN || key >= ACAP_KEY_COUNT) {
    return false;
  }

  enqueue_input((struct input_event) { .key = key, .down = data[4] != 0 });
  return true;
}

static bool
dequeue_input(struct input_event *input)
{
  pthread_mutex_lock(&input_mutex);

  if (input_count == 0) {
    pthread_mutex_unlock(&input_mutex);
    return false;
  }

  *input = input_queue[input_head];
  input_head = (input_head + 1) % INPUT_QUEUE_CAPACITY;
  input_count--;

  pthread_mutex_unlock(&input_mutex);
  return true;
}

static bool
key_event(unsigned int key, bool down, struct control_event *event)
{
  if (pressed_keys[key] == down) {
    return false;
  }

  pressed_keys[key] = down;
  int doom_key = translate_key(key);

  /* Both physical modifier keys share one DOOM key */
  for (unsigned int other = 1; other < ACAP_KEY_COUNT; other++) {
    if (other != key && pressed_keys[other] && translate_key(other) == doom_key) {
      return false;
    }
  }

  *event = (struct control_event) { .key = doom_key, .down = down };
  return true;
}

int
controls_next_event(struct control_event *event)
{
  struct input_event input;

  for (;;) {
    /* Continue a reset across calls so DOOM's event buffer cannot overflow */
    while (reset_key > 0 && reset_key < ACAP_KEY_COUNT) {
      unsigned int key = reset_key++;
      if (key_event(key, false, event)) {
        return true;
      }
    }
    reset_key = 0;

    if (!dequeue_input(&input)) {
      return false;
    }

    if (input.key == ACAP_KEY_UNKNOWN) {
      reset_key = 1;
    } else if (key_event(input.key, input.down, event)) {
      return true;
    }
  }
}
