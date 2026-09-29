#pragma once

#include <stddef.h>
#include <stdint.h>
#include "../src/sound_protocol.h"

#define SOUND_RATE 11025

void mixer_init(void);
int mixer_valid_command(const struct sound_message *message);
void mixer_command(const struct sound_message *message);
void mixer_render(int16_t *output, size_t frames);
uint32_t mixer_finished(unsigned int slot);
