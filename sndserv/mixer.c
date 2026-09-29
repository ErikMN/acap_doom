#include "sndserv.h"
#include "mixer.h"

struct mixer_channel {
  uint32_t handle;
  const uint8_t *samples;
  uint32_t length;
  uint64_t position;
  uint32_t step;
  int *left_volume;
  int *right_volume;
};

/* Only the audio callback accesses these channels after initialization */
static struct mixer_channel channels[SOUND_CHANNELS];
static uint32_t steptable[256];
static int vol_lookup[128 * 256];

void
mixer_init(void)
{
  memset(channels, 0, sizeof(channels));
  /* Init the steptable */
  for (int i = 0; i < 256; i++) {
    steptable[i] = pow(2.0, (i - 128) / 64.0) * 65536.0;
  }
  /* Init the vol_lookup */
  for (int i = 0; i < 128; i++) {
    for (int j = 0; j < 256; j++) {
      vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
    }
  }
}

int
mixer_valid_command(const struct sound_message *message)
{
  if (message->version != SOUND_PROTOCOL_VERSION || message->slot >= SOUND_CHANNELS || message->handle == 0 ||
      message->handle > INT_MAX) {
    return 0;
  }
  if (message->type == SOUND_STOP) {
    return 1;
  }
  if (message->type != SOUND_PLAY && message->type != SOUND_UPDATE) {
    return 0;
  }
  if (message->volume > 127 || message->separation > 255 || message->pitch > 255) {
    return 0;
  }
  return message->type != SOUND_PLAY || (message->id > 0 && message->id < NUMSFX);
}

static void
set_volume(struct mixer_channel *channel, unsigned int volume, unsigned int separation)
{
  /* (range: 1 - 256) */
  int sep = (int)separation + 1;
  /* (x^2 separation) */
  int left = (int)volume - ((int)volume * sep * sep) / (256 * 256);
  sep -= 257;
  /* (x^2 separation) */
  int right = (int)volume - ((int)volume * sep * sep) / (256 * 256);
  channel->left_volume = &vol_lookup[left * 256];
  channel->right_volume = &vol_lookup[right * 256];
}

void
mixer_command(const struct sound_message *message)
{
  if (!mixer_valid_command(message)) {
    return;
  }
  struct mixer_channel *channel = &channels[message->slot];
  if (message->type == SOUND_PLAY) {
    channel->handle = message->handle;
    channel->samples = S_sfx[message->id].data;
    channel->length = lengths[message->id];
    channel->position = 0;
    channel->step = (uint64_t)steptable[message->pitch] * sample_rates[message->id] / SOUND_RATE;
    set_volume(channel, message->volume, message->separation);
  } else if (channel->handle == message->handle) {
    if (message->type == SOUND_STOP) {
      channel->samples = NULL;
    } else {
      /* Preserve the starting pitch while updating the sound's position */
      set_volume(channel, message->volume, message->separation);
    }
  }
}

void
mixer_render(int16_t *output, size_t frames)
{
  /* Mix exactly the frames that will be submitted to PipeWire */
  for (size_t frame = 0; frame < frames; frame++) {
    int left = 0;
    int right = 0;
    for (unsigned int i = 0; i < SOUND_CHANNELS; i++) {
      struct mixer_channel *channel = &channels[i];
      if (!channel->samples) {
        continue;
      }
      uint64_t position = channel->position >> 16;
      if (position >= channel->length) {
        channel->samples = NULL;
        continue;
      }
      uint8_t sample = channel->samples[position];
      left += channel->left_volume[sample];
      right += channel->right_volume[sample];
      channel->position += channel->step;
      if ((channel->position >> 16) >= channel->length) {
        channel->samples = NULL;
      }
    }
    output[frame * 2] = left > INT16_MAX ? INT16_MAX : left < INT16_MIN ? INT16_MIN : left;
    output[frame * 2 + 1] = right > INT16_MAX ? INT16_MAX : right < INT16_MIN ? INT16_MIN : right;
  }
}

uint32_t
mixer_finished(unsigned int slot)
{
  return channels[slot].samples ? 0 : channels[slot].handle;
}
