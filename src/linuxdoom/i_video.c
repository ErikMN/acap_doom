// Emacs style mode select   -*- C -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
// Modified 2023 to run on Axis devices.
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <unistd.h>
#if 0
#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#endif

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_main.h"

#include "doomdef.h"

byte cur_palette[768];

#define POINTER_WARP_COUNTDOWN 1

#define MAX_CHECKED_FRAMES ((sizeof(crc_check) / sizeof(unsigned)) - 1)

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
boolean grabMouse;
int doPointerWarp = POINTER_WARP_COUNTDOWN;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
// static int multiply = 1;

void
I_ShutdownGraphics(void)
{
  // Detach from X server
  /*if (!XShmDetach(X_display, &X_shminfo))
            I_Error("XShmDetach() failed in I_ShutdownGraphics()");

  // Release shared memory.
  shmdt(X_shminfo.shmaddr);
  shmctl(X_shminfo.shmid, IPC_RMID, 0);*/

  // Paranoia.
}

//
// I_StartFrame
//
void
I_StartFrame(void)
{
  // er?
}

// static int lastmousex = 0;
// static int lastmousey = 0;
// boolean mousemoved = false;
// boolean shmFinished;

void
I_GetEvent(void)
{
  event_t event;

  typedef struct {
    int *key_press;
    int key_code;
    int debounce;
  } KeyEvent;

  /* List of key events */
  const KeyEvent key_events[] = {
    /* Arrow keys */
    { &key_press_up, KEY_UPARROW, 0 },
    { &key_press_left, KEY_LEFTARROW, 0 },
    { &key_press_right, KEY_RIGHTARROW, 0 },
    { &key_press_down, KEY_DOWNARROW, 0 },
    /* Control keys */
    { &key_press_enter, KEY_ENTER, 1 },
    { &key_press_esc, KEY_ESCAPE, 1 },
    { &key_press_ctrl, KEY_RCTRL, 0 },
    { &key_press_shift, KEY_RSHIFT, 0 },
    { &key_press_tab, KEY_TAB, 1 },
    { &key_press_space, KEY_SPACE, 0 },
    /* Character keys */
    { &key_press_comma, ',', 0 },
    { &key_press_dot, '.', 0 },
    /* Number keys */
    { &key_press_0, '0', 0 },
    { &key_press_1, '1', 0 },
    { &key_press_2, '2', 0 },
    { &key_press_3, '3', 0 },
    { &key_press_4, '4', 0 },
    { &key_press_5, '5', 0 },
    { &key_press_6, '6', 0 },
    { &key_press_7, '7', 0 },
    { &key_press_8, '8', 0 },
    { &key_press_9, '9', 0 },
    /* Letter keys */
    { &key_press_a, 'a', 1 },
    { &key_press_b, 'b', 1 },
    { &key_press_c, 'c', 1 },
    { &key_press_d, 'd', 1 },
    { &key_press_e, 'e', 1 },
    { &key_press_f, 'f', 1 },
    { &key_press_g, 'g', 1 },
    { &key_press_h, 'h', 1 },
    { &key_press_i, 'i', 1 },
    { &key_press_j, 'j', 1 },
    { &key_press_k, 'k', 1 },
    { &key_press_l, 'l', 1 },
    { &key_press_m, 'm', 1 },
    { &key_press_n, 'n', 1 },
    { &key_press_o, 'o', 1 },
    { &key_press_p, 'p', 1 },
    { &key_press_q, 'q', 1 },
    { &key_press_r, 'r', 1 },
    { &key_press_s, 's', 1 },
    { &key_press_t, 't', 1 },
    { &key_press_u, 'u', 1 },
    { &key_press_v, 'v', 1 },
    { &key_press_w, 'w', 1 },
    { &key_press_x, 'x', 1 },
    { &key_press_y, 'y', 1 },
    { &key_press_z, 'z', 1 },
    { NULL, 0, 0 },
  };

  for (int i = 0; key_events[i].key_press != NULL; i++) {
    if (*(key_events[i].key_press)) {
      /* Key press */
      event.type = ev_keydown;
      event.data1 = key_events[i].key_code;
      event.data2 = 0;
      event.data3 = 0;
      /* Debounce key */
      if (key_events[i].debounce) {
        *(key_events[i].key_press) = 0;
      }
    } else {
      /* Key release */
      event.type = ev_keyup;
      event.data1 = key_events[i].key_code;
      event.data2 = 0;
      event.data3 = 0;
    }
    D_PostEvent(&event);
  }
}

//
// I_StartTic
//
void
I_StartTic(void)
{
  /* Poll for events (calling I_GetEvent while events are in
   * the queue) and stop the mouse pointer leaving the window
   * as a dirty keep-focus hack
   */
  I_GetEvent();
}

//
// I_UpdateNoBlit
//
void
I_UpdateNoBlit(void)
{
  // what is this?
}

void
SaveRGBScreenshot(unsigned char *buffer)
{
  static int counter = 0;
  char filename[64];
  snprintf(filename, sizeof(filename), "/tmp/doom/screenshot%d.rgb", counter++);

  FILE *file = fopen(filename, "wb");
  if (!file)
    return;

  // Convert and write out as 24-bit RGB
  for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++) {
    unsigned char index = buffer[i];
    fwrite(&cur_palette[index * 3], 3, 1, file); // Write RGB triple
  }

  fclose(file);
}

void
CaptureRGBABuffer(void)
{
  unsigned char *target_buffer = rgba_buffer;
  unsigned char *screen_ptr = screens[0];
  unsigned char *palette_ptr = cur_palette;
  /* Convert and populate the buffer as 32-bit RGBA */
  for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++) {
    unsigned char index = *screen_ptr++;
    *target_buffer++ = palette_ptr[index * 3];     /* Red */
    *target_buffer++ = palette_ptr[index * 3 + 1]; /* Green */
    *target_buffer++ = palette_ptr[index * 3 + 2]; /* Blue */
    *target_buffer++ = 255;                        /* Alpha */
  }
}

//
// I_FinishUpdate
//
void
I_FinishUpdate(void)
{
  // printf("*** I_FinishUpdate\n");
  // SaveRGBScreenshot(screens[0]);

  /* NOTE: capture the buffer and redraw (render_trigger_redraw) */
  CaptureRGBABuffer();
}

//
// I_ReadScreen
//
void
I_ReadScreen(byte *scr)
{
  memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// Palette stuff.
//

//
// I_SetPalette
//
void
I_SetPalette(byte *palette)
{
  memcpy(cur_palette, palette, 768);
  // palette is an array of 256 RGB triples.
  // i.e. 768 bytes
}

void
I_InitGraphics(void)
{

  char *displayname;
  (void)displayname;
  // char *d;
  // int n;
  int pnum;
  // int x = 0;
  // int y = 0;

  // warning: char format, different type arg
  // char xsign = ' ';
  // char ysign = ' ';

  // int oktodraw;
  // unsigned long attribmask;
  // int valuemask;
  static int firsttime = 1;

  if (!firsttime)
    return;
  firsttime = 0;

#if 0
    signal(SIGINT, (void (*)(int)) I_Quit);
#endif

  // check for command-line display name
  if ((pnum = M_CheckParm("-disp"))) // suggest parentheses around assignment
    displayname = myargv[pnum + 1];
  else
    displayname = 0;

  // check if the user wants to grab the mouse (quite unnice)
  grabMouse = !!M_CheckParm("-grabmouse");

  screens[0] = (unsigned char *)malloc(SCREENWIDTH * SCREENHEIGHT);
}
