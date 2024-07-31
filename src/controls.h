/*
 * Sets key states in I_GetEvent():
 * src/linuxdoom/i_video.c
 */
#pragma once

#include <stdio.h>
#include <string.h>

/* Global key states */
extern int key_press_up;
extern int key_press_left;
extern int key_press_right;
extern int key_press_down;
extern int key_press_enter;
extern int key_press_esc;
extern int key_press_ctrl;
extern int key_press_shift;
extern int key_press_dot;
extern int key_press_comma;
extern int key_press_space;
extern int key_press_tab;

extern int key_press_0;
extern int key_press_1;
extern int key_press_2;
extern int key_press_3;
extern int key_press_4;
extern int key_press_5;
extern int key_press_6;
extern int key_press_7;
extern int key_press_8;
extern int key_press_9;

extern int key_press_a;
extern int key_press_b;
extern int key_press_c;
extern int key_press_d;
extern int key_press_e;
extern int key_press_f;
extern int key_press_g;
extern int key_press_h;
extern int key_press_i;
extern int key_press_j;
extern int key_press_k;
extern int key_press_l;
extern int key_press_m;
extern int key_press_n;
extern int key_press_o;
extern int key_press_p;
extern int key_press_q;
extern int key_press_r;
extern int key_press_s;
extern int key_press_t;
extern int key_press_u;
extern int key_press_v;
extern int key_press_w;
extern int key_press_x;
extern int key_press_y;
extern int key_press_z;

void set_key_state(const char *key_str);
