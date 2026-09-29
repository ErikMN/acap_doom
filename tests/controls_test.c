#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#include "controls.h"
#include "linuxdoom/d_event.h"
#include "linuxdoom/doomdef.h"

event_t events[MAXEVENTS];
int eventhead;
int eventtail;

void I_GetEvent(void);

void
D_PostEvent(event_t *event)
{
  int next = (eventhead + 1) & (MAXEVENTS - 1);
  assert(next != eventtail);
  events[eventhead] = *event;
  eventhead = next;
}

static void
send_key(unsigned int key, int down)
{
  uint8_t packet[] = { 1, 1, key & 255, key >> 8, down };
  assert(controls_receive(packet, sizeof(packet)));
}

static void
expect_key(int key, int down)
{
  struct control_event event;
  assert(controls_next_event(&event));
  assert(event.key == key);
  assert(event.down == down);
}

static void
expect_empty(void)
{
  struct control_event event;
  assert(!controls_next_event(&event));
}

static void
test_packets(void)
{
  uint8_t packet[] = { 1, 1, ACAP_KEY_ENTER, 0, 1, 0 };
  assert(!controls_receive(NULL, 0));
  for (size_t length = 0; length <= sizeof(packet); length++) {
    if (length != 5) {
      assert(!controls_receive(packet, length));
    }
  }
  packet[0] = 2;
  assert(!controls_receive(packet, 5));
  packet[0] = 1;
  packet[1] = 2;
  assert(!controls_receive(packet, 5));
  packet[1] = 1;
  packet[2] = 0;
  assert(!controls_receive(packet, 5));
  packet[2] = ACAP_KEY_COUNT;
  assert(!controls_receive(packet, 5));
  packet[2] = ACAP_KEY_ENTER;
  packet[3] = 1;
  assert(!controls_receive(packet, 5));
  packet[3] = 0;
  packet[4] = 2;
  assert(!controls_receive(packet, 5));
  expect_empty();

  send_key(ACAP_KEY_ENTER, 1);
  send_key(ACAP_KEY_ENTER, 0);
  expect_key(KEY_ENTER, 1);
  expect_key(KEY_ENTER, 0);
  send_key(ACAP_KEY_COMMA, 1);
  send_key(ACAP_KEY_PERIOD, 1);
  expect_key(',', 1);
  expect_key('.', 1);
  const uint8_t reset[] = { 1, 5 };
  assert(controls_receive(reset, sizeof(reset)));
  expect_key(',', 0);
  expect_key('.', 0);
  expect_empty();
}

static void
test_modifiers(void)
{
  const unsigned int left[] = { ACAP_KEY_SHIFT_LEFT, ACAP_KEY_CTRL_LEFT, ACAP_KEY_ALT_LEFT };
  const unsigned int right[] = { ACAP_KEY_SHIFT_RIGHT, ACAP_KEY_CTRL_RIGHT, ACAP_KEY_ALT_RIGHT };
  const int doom[] = { KEY_RSHIFT, KEY_RCTRL, KEY_RALT };
  for (size_t i = 0; i < 3; i++) {
    send_key(left[i], 1);
    send_key(left[i], 1);
    send_key(right[i], 1);
    send_key(left[i], 0);
    expect_key(doom[i], 1);
    expect_empty();
    send_key(right[i], 0);
    expect_key(doom[i], 0);
    expect_empty();
  }
}

static void
test_reset_and_overflow(void)
{
  send_key(ACAP_KEY_ARROW_UP, 1);
  expect_key(KEY_UPARROW, 1);
  send_key(ACAP_KEY_A, 1);
  controls_reset();
  expect_key(KEY_UPARROW, 0);
  expect_empty();

  send_key(ACAP_KEY_CTRL_LEFT, 1);
  expect_key(KEY_RCTRL, 1);
  for (int i = 0; i < 257; i++) {
    send_key(ACAP_KEY_A, (i & 1) == 0);
  }
  expect_key(KEY_RCTRL, 0);
  expect_empty();
  send_key(ACAP_KEY_B, 1);
  send_key(ACAP_KEY_B, 0);
  expect_key('b', 1);
  expect_key('b', 0);
}

static void
test_doom_buffer(void)
{
  for (int i = 0; i < 100; i++) {
    send_key(ACAP_KEY_A, (i & 1) == 0);
  }
  I_GetEvent();
  assert(eventhead == MAXEVENTS - 1);
  I_GetEvent();
  assert(eventhead == MAXEVENTS - 1);
  int count = 0;
  while (count < 100) {
    while (eventtail != eventhead) {
      assert(events[eventtail].type == ((count & 1) == 0 ? ev_keydown : ev_keyup));
      assert(events[eventtail].data1 == 'a');
      assert(events[eventtail].data2 == 0 && events[eventtail].data3 == 0);
      eventtail = (eventtail + 1) & (MAXEVENTS - 1);
      count++;
    }
    I_GetEvent();
  }
  expect_empty();
}

static atomic_int producer_done;

static void *
produce_input(void *unused)
{
  (void)unused;
  for (int i = 0; i < 10000; i++) {
    send_key(ACAP_KEY_A, 1);
    send_key(ACAP_KEY_A, 0);
  }
  controls_reset();
  atomic_store(&producer_done, 1);
  return NULL;
}

static void
test_threads(void)
{
  pthread_t producer;
  struct control_event event;
  int down = 0;
  assert(pthread_create(&producer, NULL, produce_input, NULL) == 0);
  for (;;) {
    int done = atomic_load(&producer_done);
    while (controls_next_event(&event)) {
      assert(event.key == 'a');
      assert(event.down != down);
      down = event.down;
    }
    if (done) {
      break;
    }
  }
  assert(pthread_join(producer, NULL) == 0);
  assert(!down);
}

int
main(void)
{
  test_packets();
  test_modifiers();
  test_reset_and_overflow();
  test_doom_buffer();
  test_threads();
  puts("PASS: packets, modifiers, reset, overflow, DOOM event buffer, concurrent input");

  return 0;
}
