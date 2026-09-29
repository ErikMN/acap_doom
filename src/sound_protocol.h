#pragma once

#include <stdint.h>

#define SOUND_PROTOCOL_VERSION 1
#define SOUND_CHANNELS 8

/* Fixed-size messages between local processes on the same device */
enum sound_message_type { SOUND_PLAY = 1, SOUND_STOP, SOUND_UPDATE, SOUND_FINISHED };

struct sound_message {
  uint32_t version;
  uint32_t type;
  uint32_t slot;
  uint32_t handle;
  uint32_t id;
  uint32_t volume;
  uint32_t separation;
  uint32_t pitch;
};
