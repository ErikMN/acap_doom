/**
 * Linux Doom Sound Server
 *
 * This is the original Linux Doom Sound Server ported from OSS to PipeWire
 * for modern Linux systems.
 *
 */
#include "sndserv.h"

#define DEFAULT_RATE 11025
#define DEFAULT_CHANNELS 2
#define NBR_OF_CHANNELS 8

/* pw-top: QUANT/RATE */
#ifdef HOST
#define BUFFER_LATENCY "256/11025"
#else
#define BUFFER_LATENCY "512/11025"
#endif

struct data {
  struct pw_thread_loop *loop;
  struct pw_stream *stream;
  struct pw_context *context;
  struct pw_core *core;
  struct spa_hook stream_listener;
};

struct ThreadArgs {
  int argc;
  char **argv;
  char commandbuf[256];
  struct data data;
};

struct data data = { 0 };

static int start_stream(struct data *data);
static void stop_stream(struct data *data);
static void mix_audio(void);

/******************************************************************************/
/* GLOBAL VARIABLES */

/* sndserver running flag */
bool is_running = true;

/* Global clip id */
static int g_fx_id = 0;

/* Channel id */
static int slot = 0;

/* the channel data pointers */
static uint8_t *channels[NBR_OF_CHANNELS];

/* the channel step amount */
static unsigned int channelstep[NBR_OF_CHANNELS];

/* 0.16 bit remainder of last step */
static unsigned int channelstepremainder[NBR_OF_CHANNELS];

/* the channel data end pointers */
static uint8_t *channelsend[NBR_OF_CHANNELS];

/* time that the channel started playing */
static long channelstart[NBR_OF_CHANNELS];

/* the channel left volume lookup */
static int *channelleftvol_lookup[NBR_OF_CHANNELS];

/* the channel right volume lookup */
static int *channelrightvol_lookup[NBR_OF_CHANNELS];

/* sfx id of the playing sound effect */
static int channelids[NBR_OF_CHANNELS];

/* A table of steps (sound speeds) */
static int steptable[256];

/* A table of volumes to use */
static int vol_lookup[128 * 256];

/* Mixing buffer (the one sent to OSS in the original code) */
static int16_t mixbuffer[MIXBUFFERSIZE];

/******************************************************************************/
/* SOUNDSERVER FUNCTIONS */

/* DEBUG: Dump raw sound clip to file */
NOT_USED static void
dump_channel_to_file(const char *filename)
{
  FILE *file = fopen(filename, "wb");
  if (file) {
    fwrite(channels[slot], sizeof(uint8_t), lengths[g_fx_id], file);
    fclose(file);
    PRINT_BLUE("*** Dumped channel of sound %d to %s length: %d", g_fx_id, filename, lengths[g_fx_id]);
  } else {
    perror("Failed to open file for writing");
  }
}

/* Mix audio to the mixbuffer */
static void
mix_audio(void)
{
  // PRINT_YELLOW("MIX AUDIO");
  /* On most platforms, a short integer is 16 bits */
  uint8_t sample;    /* Current sample value */
  int16_t *leftout;  /* Left ch output buffer */
  int16_t *rightout; /* Right ch output buffer */
  int16_t *leftend;  /* Pointer to the end of the left ch output buffer*/
  int step;          /* Step through the buffer */

  leftout = mixbuffer;
  rightout = mixbuffer + 1;
  step = 2; /* each ch samples are 2 positions apart in the buffer */

  leftend = mixbuffer + SAMPLECOUNT * step;

  /* Mix into the mixing buffer */
  while (leftout != leftend) {
    int dl = 0;
    int dr = 0;

    /* For each channel */
    for (int i = 0; i < NBR_OF_CHANNELS; i++) {
      if (channels[i]) {
        sample = *channels[i];

        dl += channelleftvol_lookup[i][sample];
        dr += channelrightvol_lookup[i][sample];

        channelstepremainder[i] += channelstep[i];
        channels[i] += channelstepremainder[i] >> 16;
        channelstepremainder[i] &= UINT16_MAX;

        if (channels[i] >= channelsend[i]) {
          channels[i] = NULL;
        }
      }
    }

    if (dl > INT16_MAX)
      *leftout = INT16_MAX;
    else if (dl < INT16_MIN)
      *leftout = INT16_MIN;
    else
      *leftout = dl;

    if (dr > INT16_MAX)
      *rightout = INT16_MAX;
    else if (dr < INT16_MIN)
      *rightout = INT16_MIN;
    else
      *rightout = dr;

    leftout += step;
    rightout += step;
  }
}

void
I_SubmitOutputBuffer(void *samples, int samplecount)
{
  (void)samples;
  (void)samplecount;
  PRINT_CYAN("DO SOUND: samples: %p samplecount: %d", samples, samplecount);
}

static long
get_current_time_in_microseconds(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

/* Add a sound to the buffer */
static void
addsfx(int sfxid, int volume, int step, int separation)
{
  PRINT_YELLOW("[sfxid: %d] [volume: %d] [step: %d] [separation: %d]", sfxid, volume, step, separation);

  /* FIXME: Sanity check values */
  if (sfxid < 0 || sfxid > NUMSFX) {
    PRINT_RED("Error: invalid sfx ID");
    return;
  }
  if (separation < 1 || separation > 256) {
    separation = 0;
  }
  if (volume < 0 || volume > 0xff) {
    PRINT_RED("Volume out of bounds!");
    volume = 8;
  }
  // if (step < 16384 || step > 46083) {
  //   PRINT_RED("Step out of bounds!");
  //   step = 66000;
  // }

  int i;
  long mytime = get_current_time_in_microseconds();
  long oldest = mytime;
  int oldestnum = 0;
  int rightvol = 0;
  int leftvol = 0;

  PRINT_GREEN("mytime: %ld", mytime);

  /* Play these sound effects only one at a time */
  if (sfxid == sfx_sawup || sfxid == sfx_sawidl || sfxid == sfx_sawful || sfxid == sfx_sawhit || sfxid == sfx_stnmov ||
      sfxid == sfx_pistol) {
    PRINT_YELLOW("Only play this sound once!");
    for (i = 0; i < NBR_OF_CHANNELS; i++) {
      if (channels[i] && channelids[i] == sfxid) {
        channels[i] = NULL;
        break;
      }
    }
  }

  for (i = 0; i < NBR_OF_CHANNELS && channels[i]; i++) {
    if (channelstart[i] < oldest) {
      oldestnum = i;
      oldest = channelstart[i];
    }
  }

  if (i == NBR_OF_CHANNELS) {
    slot = oldestnum;
  } else {
    slot = i;
  }

  PRINT_YELLOW("CHANNEL SLOT: %d", slot);

  /* Add sound data to a channel */
  channels[slot] = (uint8_t *)S_sfx[sfxid].data;
  channelsend[slot] = channels[slot] + lengths[sfxid];

  // dump_channel_to_file("channel.raw");

  channelstep[slot] = step;
  channelstepremainder[slot] = 0;
  channelstart[slot] = mytime;

  /* (range: 1 - 256) */
  separation += 1;

  /* (x^2 separation) */
  leftvol = volume - (volume * separation * separation) / (256 * 256);

  separation = separation - 257;

  /* (x^2 separation) */
  rightvol = volume - (volume * separation * separation) / (256 * 256);

  /* sanity check */
  if (rightvol < 0 || rightvol > 127) {
    PRINT_RED("rightvol out of bounds");
    rightvol = 50;
  }

  if (leftvol < 0 || leftvol > 127) {
    PRINT_RED("leftvol out of bounds");
    leftvol = 50;
  }

  /* Get the proper lookup table piece for this volume level */
  channelleftvol_lookup[slot] = &vol_lookup[leftvol * 256];
  channelrightvol_lookup[slot] = &vol_lookup[rightvol * 256];
  channelids[slot] = sfxid;
}

/* Initialize sound server internal data */
static void
initdata(void)
{
  /* Init all channel pointers to 0 */
  memset(channels, 0, sizeof(channels));

  /* Init the steptable */
  int *steptablemid = steptable + 128;
  for (int i = -128; i < 128; i++) {
    steptablemid[i] = pow(2.0, i / 64.0) * 65536.0;
  }
  /* Init the vol_lookup */
  for (int i = 0; i < 128; i++) {
    for (int j = 0; j < 256; j++) {
      vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
    }
  }
}

/* Sound server thread loop */
static void *
sndserver(void *arg)
{
  struct ThreadArgs *threadArgs = (struct ThreadArgs *)arg;
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    return NULL;
  }
  struct epoll_event ev, events[1];
  ev.events = EPOLLIN;
  ev.data.fd = 0;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &ev) == -1) {
    perror("epoll_ctl");
    close(epoll_fd);
    return NULL;
  }

  /* Get sound data from WAD */
  grabdata();

  /* Init any data */
  initdata();

  while (is_running) {
    char *commandbuf = threadArgs->commandbuf;
    int rc = epoll_wait(epoll_fd, events, 1, -1);
    if (rc < 0) {
      perror("epoll_wait");
      is_running = false;
      pw_thread_loop_signal(threadArgs->data.loop, false);
    }
    if (events[0].events & EPOLLIN) {
      ssize_t r = read(0, commandbuf, 1);
      if (r <= 0) {
        is_running = false;
        pw_thread_loop_signal(threadArgs->data.loop, false);
      } else {
        char cmd = commandbuf[0];
        if (cmd != '\n') {
          PRINT_BLUE("CMD: %c", cmd);
        }
        switch (cmd) {
        case 'p':
          /* Parse play command */
          r = read(0, commandbuf, 9);
          if (r <= 0) {
            is_running = false;
            pw_thread_loop_signal(threadArgs->data.loop, false);
          }
          for (int i = 0; i < 8; i++) {
            char offset = commandbuf[i] >= 'a' ? 'a' - 10 : '0';
            commandbuf[i] -= offset;
          }
          int sndnum = (commandbuf[0] << 4) + commandbuf[1];
          int step_index = (commandbuf[2] << 4) + commandbuf[3];
          int step = steptable[step_index];
          int volume = (commandbuf[4] << 4) + commandbuf[5];
          int separation = (commandbuf[6] << 4) + commandbuf[7];

          PRINT_CYAN("p %x %x %x %x", sndnum, step_index, volume, separation);
          PRINT_CYAN(
              "[sndnum: %d] [volume: %d] [step_index: %d]  [separation: %d]", sndnum, volume, step_index, separation);
          PRINT_CYAN("[sndnum: %#x] [volume: %#x] [step_index: %#x]  [separation: %#x]",
                     sndnum,
                     volume,
                     step_index,
                     separation);

          g_fx_id = sndnum;
          addsfx(sndnum, volume, step, separation);
          break;

        /* Quit app */
        case 'q':
          is_running = false;
          pw_thread_loop_signal(threadArgs->data.loop, false);
          break;

        case '\n':
          break;

        default:
          PRINT_RED("Invalid cmd: %c", commandbuf[0]);
          break;
        }
      }
    }
  }
  close(epoll_fd);

  return NULL;
}

/******************************************************************************/
/* PIPEWIRE */

/* Process audio buffer */
static void
on_process(void *userdata)
{
  /* Mix audio */
  mix_audio();

  struct data *data = userdata;
  struct pw_buffer *pw_buffer = NULL;
  struct spa_buffer *buf = NULL;
  struct spa_data *spa_data = NULL;
  struct spa_chunk *chunk = NULL;
  int n_frames = 0;
  int stride = 0;
  int16_t *p;

  if ((pw_buffer = pw_stream_dequeue_buffer(data->stream)) == NULL) {
    PRINT_RED("out of buffers");
    return;
  }

  buf = pw_buffer->buffer;
  spa_data = buf->datas;
  chunk = spa_data->chunk;
  buf = pw_buffer->buffer;
  if ((p = spa_data[0].data) == NULL) {
    return;
  }

  /* length of clip */
  int length = MIXBUFFERSIZE;
  stride = sizeof(int16_t) * DEFAULT_CHANNELS;

  n_frames = buf->datas[0].maxsize / stride;
  if (pw_buffer->requested)
    n_frames = SPA_MIN(pw_buffer->requested, n_frames);

  // syslog(LOG_INFO, "length: %d n_frames %d stride: %d slot: %d", length, n_frames, stride, slot);

  memcpy(p, mixbuffer, length);

  chunk->offset = 0;
  chunk->stride = stride;
  chunk->size = n_frames * stride;

  pw_stream_queue_buffer(data->stream, pw_buffer);
}

static const struct pw_stream_events stream_events = {
  PW_VERSION_STREAM_EVENTS,
  .process = on_process,
};

static void
do_quit(void *userdata, int signal_number)
{
  PRINT_BLUE("QUIT!");
  (void)signal_number;
  struct data *data = userdata;
  /* Stop the PW loop */
  pw_thread_loop_lock(data->loop);
  stop_stream(data);
  pw_thread_loop_signal(data->loop, false);
  pw_thread_loop_unlock(data->loop);
}

static void
stop_stream(struct data *data)
{
  PRINT_BLUE(">>> Stop stream!");
  if (data->stream == NULL) {
    PRINT_RED("Stream is already stopped!");
    return;
  }
  /* Stop the stream */
  pw_stream_destroy(data->stream);
  data->stream = NULL;
}

static int
start_stream(struct data *data)
{
  PRINT_BLUE(">>> Start stream!");
  if (data->stream != NULL) {
    PRINT_RED("Stream is already running!");
    return 0;
  }

  uint8_t buffer[1024];
  const struct spa_pod *params[1];
  struct pw_properties *props;
  struct spa_pod_builder pod_builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

  int ret = 0;

  /* Set the target node */
  props = pw_properties_new(PW_KEY_MEDIA_TYPE,
                            "Audio",
                            PW_KEY_MEDIA_CATEGORY,
                            "Playback",
                            PW_KEY_TARGET_OBJECT,
                            "AudioDevice0Input0",
                            PW_KEY_NODE_LATENCY,
                            BUFFER_LATENCY,
                            NULL);

  /* Create a new stream */
  data->stream = pw_stream_new(data->core, "DOOM sound server", props);

  struct spa_audio_info_raw raw =
      SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_S16_LE, .channels = DEFAULT_CHANNELS, .rate = DEFAULT_RATE);
  params[0] = spa_format_audio_raw_build(&pod_builder, SPA_PARAM_EnumFormat, &raw);

  pw_stream_add_listener(data->stream, &data->stream_listener, &stream_events, data);

  /* Connect the stream */
  ret = pw_stream_connect(data->stream,
                          PW_DIRECTION_OUTPUT,
                          PW_ID_ANY, /* link to any node */
                          PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS,
                          params,
                          SPA_N_ELEMENTS(params));
  if (ret < 0) {
    printf("Error connecting to stream\n");
  }

  return ret;
}

/******************************************************************************/
/* MAIN */

int
main(int argc, char *argv[])
{
  printf("*** Starting Linux Doom Sound Server [version: %s]\n", SNDSERV_VERSION);
  PRINT_BLUE("Build: %s %s\n", __DATE__, __TIME__);
  bool ret = false;

  /* Initialize PipeWire */
  pw_init(&argc, &argv);

  /* Print headers and library version */
  print_debug("Compiled with libpipewire %s headers\n"
              "Linked with libpipewire %s library\n\n",
              pw_get_headers_version(),
              pw_get_library_version());

  /* Create a new PW thread loop */
  data.loop = pw_thread_loop_new("Doom sndserv PW thread", NULL);
  if (!data.loop) {
    PRINT_RED("Could not create the main loop");
    ret = false;
    goto exit;
  }

  /* Lock the loop */
  pw_thread_loop_lock(data.loop);

  /* Setup signals */
  pw_loop_add_signal(pw_thread_loop_get_loop(data.loop), SIGINT, do_quit, &data);
  pw_loop_add_signal(pw_thread_loop_get_loop(data.loop), SIGTERM, do_quit, &data);

  /* Create the context */
  data.context = pw_context_new(pw_thread_loop_get_loop(data.loop), NULL, 0);
  /* Connect to a PipeWire instance */
  data.core = pw_context_connect(data.context, NULL, 0);
  if (data.core == NULL) {
    fprintf(stderr, "can't connect: %m\n");
    goto exit;
  }

  /* Start the stream */
  start_stream(&data);

  /* sndserver input args */
  struct ThreadArgs threadArgs;
  threadArgs.argc = argc;
  threadArgs.argv = argv;
  threadArgs.data = data;

  /* Start the sndserver thread */
  pthread_t sndserv_thread;
  if (pthread_create(&sndserv_thread, NULL, sndserver, (void *)&threadArgs) != 0) {
    perror("pthread_create");
    goto exit;
  }

  /* Start the PW loop: the app stops here */
  pw_thread_loop_start(data.loop);
  pw_thread_loop_wait(data.loop);

  /* Unlock loop before stop */
  pw_thread_loop_unlock(data.loop);
  pw_thread_loop_stop(data.loop);

  /* Stop the sndserver thread loop */
  is_running = false;

  /* FIXME: Join the sndserver thread */
  // if (pthread_join(sndserv_thread, NULL) != 0) {
  //   perror("pthread_join");
  //   goto exit;
  // }

  ret = true;

exit:
  /* Clean up PW */
  if (data.loop != NULL) {
    PRINT_YELLOW("Destroy the loop");
    pw_thread_loop_destroy(data.loop);
  }
  pw_deinit();
  PRINT_BLUE("*** EXIT APP: BYE");

  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
