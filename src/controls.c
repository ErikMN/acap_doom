#include "controls.h"

/* Key states */
int key_press_up = 0;
int key_press_left = 0;
int key_press_right = 0;
int key_press_down = 0;
int key_press_enter = 0;
int key_press_esc = 0;
int key_press_ctrl = 0;
int key_press_shift = 0;
int key_press_dot = 0;
int key_press_comma = 0;
int key_press_space = 0;
int key_press_tab = 0;

int key_press_0 = 0;
int key_press_1 = 0;
int key_press_2 = 0;
int key_press_3 = 0;
int key_press_4 = 0;
int key_press_5 = 0;
int key_press_6 = 0;
int key_press_7 = 0;
int key_press_8 = 0;
int key_press_9 = 0;

int key_press_a = 0;
int key_press_b = 0;
int key_press_c = 0;
int key_press_d = 0;
int key_press_e = 0;
int key_press_f = 0;
int key_press_g = 0;
int key_press_h = 0;
int key_press_i = 0;
int key_press_j = 0;
int key_press_k = 0;
int key_press_l = 0;
int key_press_m = 0;
int key_press_n = 0;
int key_press_o = 0;
int key_press_p = 0;
int key_press_q = 0;
int key_press_r = 0;
int key_press_s = 0;
int key_press_t = 0;
int key_press_u = 0;
int key_press_v = 0;
int key_press_w = 0;
int key_press_x = 0;
int key_press_y = 0;
int key_press_z = 0;

typedef struct {
  const char *key_str;
  int *key_state;
  int state_value;
} KeyAction;

void
set_key_state(const char *key_str)
{
  /* List of key actions */
  static KeyAction key_actions[] = {
    /* Arrow keys */
    { "ArrowUp", &key_press_up, 1 },
    { "ArrowUp_release", &key_press_up, 0 },
    { "ArrowLeft", &key_press_left, 1 },
    { "ArrowLeft_release", &key_press_left, 0 },
    { "ArrowRight", &key_press_right, 1 },
    { "ArrowRight_release", &key_press_right, 0 },
    { "ArrowDown", &key_press_down, 1 },
    { "ArrowDown_release", &key_press_down, 0 },
    /* Control keys */
    { "Enter", &key_press_enter, 1 },
    { "Enter_release", &key_press_enter, 0 },
    { "Escape", &key_press_esc, 1 },
    { "Escape_release", &key_press_esc, 0 },
    { "Control", &key_press_ctrl, 1 },
    { "Control_release", &key_press_ctrl, 0 },
    { "Shift", &key_press_shift, 1 },
    { "Shift_release", &key_press_shift, 0 },
    { "Tab", &key_press_tab, 1 },
    { "Tab_release", &key_press_tab, 0 },
    /* Character keys */
    { ",", &key_press_comma, 1 },
    { ",_release", &key_press_comma, 0 },
    { ".", &key_press_dot, 1 },
    { "._release", &key_press_dot, 0 },
    { " ", &key_press_space, 1 },
    { " _release", &key_press_space, 0 },
    /* Number keys */
    { "0", &key_press_0, 1 },
    { "0_release", &key_press_0, 0 },
    { "1", &key_press_1, 1 },
    { "1_release", &key_press_1, 0 },
    { "2", &key_press_2, 1 },
    { "2_release", &key_press_2, 0 },
    { "3", &key_press_3, 1 },
    { "3_release", &key_press_3, 0 },
    { "4", &key_press_4, 1 },
    { "4_release", &key_press_4, 0 },
    { "5", &key_press_5, 1 },
    { "5_release", &key_press_5, 0 },
    { "6", &key_press_6, 1 },
    { "6_release", &key_press_6, 0 },
    { "7", &key_press_7, 1 },
    { "7_release", &key_press_7, 0 },
    { "8", &key_press_8, 1 },
    { "8_release", &key_press_8, 0 },
    { "9", &key_press_9, 1 },
    { "9_release", &key_press_9, 0 },
    /* Letter keys */
    { "a", &key_press_a, 1 },
    { "a_release", &key_press_a, 0 },
    { "b", &key_press_b, 1 },
    { "b_release", &key_press_b, 0 },
    { "c", &key_press_c, 1 },
    { "c_release", &key_press_c, 0 },
    { "d", &key_press_d, 1 },
    { "d_release", &key_press_d, 0 },
    { "e", &key_press_e, 1 },
    { "e_release", &key_press_e, 0 },
    { "f", &key_press_f, 1 },
    { "f_release", &key_press_f, 0 },
    { "g", &key_press_g, 1 },
    { "g_release", &key_press_g, 0 },
    { "h", &key_press_h, 1 },
    { "h_release", &key_press_h, 0 },
    { "i", &key_press_i, 1 },
    { "i_release", &key_press_i, 0 },
    { "j", &key_press_j, 1 },
    { "j_release", &key_press_j, 0 },
    { "k", &key_press_k, 1 },
    { "k_release", &key_press_k, 0 },
    { "l", &key_press_l, 1 },
    { "l_release", &key_press_l, 0 },
    { "m", &key_press_m, 1 },
    { "m_release", &key_press_m, 0 },
    { "n", &key_press_n, 1 },
    { "n_release", &key_press_n, 0 },
    { "o", &key_press_o, 1 },
    { "o_release", &key_press_o, 0 },
    { "p", &key_press_p, 1 },
    { "p_release", &key_press_p, 0 },
    { "q", &key_press_q, 1 },
    { "q_release", &key_press_q, 0 },
    { "r", &key_press_r, 1 },
    { "r_release", &key_press_r, 0 },
    { "s", &key_press_s, 1 },
    { "s_release", &key_press_s, 0 },
    { "t", &key_press_t, 1 },
    { "t_release", &key_press_t, 0 },
    { "u", &key_press_u, 1 },
    { "u_release", &key_press_u, 0 },
    { "v", &key_press_v, 1 },
    { "v_release", &key_press_v, 0 },
    { "w", &key_press_w, 1 },
    { "w_release", &key_press_w, 0 },
    { "x", &key_press_x, 1 },
    { "x_release", &key_press_x, 0 },
    { "y", &key_press_y, 1 },
    { "y_release", &key_press_y, 0 },
    { "z", &key_press_z, 1 },
    { "z_release", &key_press_z, 0 },
    { NULL, NULL, 0 },
  };
  /* Set key states */
  for (int i = 0; key_actions[i].key_str != NULL; i++) {
    if (strcmp(key_str, key_actions[i].key_str) == 0) {
      *(key_actions[i].key_state) = key_actions[i].state_value;
      return;
    }
  }
  // syslog(LOG_INFO, "Unknown key_str: %s", key_str);
}
