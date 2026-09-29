#pragma once

#include <stddef.h>
#include <stdint.h>

struct control_event {
  int key;
  int down;
};

/* Protocol identifiers match AcapKey in KeyPressHandler.tsx */
enum acap_key_code {
  ACAP_KEY_UNKNOWN = 0,

  ACAP_KEY_A = 1,
  ACAP_KEY_B = 2,
  ACAP_KEY_C = 3,
  ACAP_KEY_D = 4,
  ACAP_KEY_E = 5,
  ACAP_KEY_F = 6,
  ACAP_KEY_G = 7,
  ACAP_KEY_H = 8,
  ACAP_KEY_I = 9,
  ACAP_KEY_J = 10,
  ACAP_KEY_K = 11,
  ACAP_KEY_L = 12,
  ACAP_KEY_M = 13,
  ACAP_KEY_N = 14,
  ACAP_KEY_O = 15,
  ACAP_KEY_P = 16,
  ACAP_KEY_Q = 17,
  ACAP_KEY_R = 18,
  ACAP_KEY_S = 19,
  ACAP_KEY_T = 20,
  ACAP_KEY_U = 21,
  ACAP_KEY_V = 22,
  ACAP_KEY_W = 23,
  ACAP_KEY_X = 24,
  ACAP_KEY_Y = 25,
  ACAP_KEY_Z = 26,

  ACAP_KEY_DIGIT_0 = 27,
  ACAP_KEY_DIGIT_1 = 28,
  ACAP_KEY_DIGIT_2 = 29,
  ACAP_KEY_DIGIT_3 = 30,
  ACAP_KEY_DIGIT_4 = 31,
  ACAP_KEY_DIGIT_5 = 32,
  ACAP_KEY_DIGIT_6 = 33,
  ACAP_KEY_DIGIT_7 = 34,
  ACAP_KEY_DIGIT_8 = 35,
  ACAP_KEY_DIGIT_9 = 36,

  ACAP_KEY_ESCAPE = 37,
  ACAP_KEY_TAB = 38,
  ACAP_KEY_ENTER = 39,
  ACAP_KEY_SPACE = 40,
  ACAP_KEY_BACKSPACE = 41,
  ACAP_KEY_SHIFT_LEFT = 42,
  ACAP_KEY_SHIFT_RIGHT = 43,
  ACAP_KEY_CTRL_LEFT = 44,
  ACAP_KEY_CTRL_RIGHT = 45,
  ACAP_KEY_ALT_LEFT = 46,
  ACAP_KEY_ALT_RIGHT = 47,
  ACAP_KEY_ARROW_UP = 48,
  ACAP_KEY_ARROW_DOWN = 49,
  ACAP_KEY_ARROW_LEFT = 50,
  ACAP_KEY_ARROW_RIGHT = 51,

  ACAP_KEY_COMMA = 52,
  ACAP_KEY_PERIOD = 53,

  ACAP_KEY_COUNT = 54,
};

/* Accept a complete binary key or reset message from the WebSocket thread */
int controls_receive(const uint8_t *data, size_t len);

/* Discard pending input and release held keys on the game thread */
void controls_reset(void);

/* Read the next DOOM event on the game thread */
int controls_next_event(struct control_event *event);
