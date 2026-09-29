/**
 * Linux Doom Sound Server
 *
 * This is the original Linux Doom Sound Server ported from OSS to PipeWire
 * for modern Linux systems.
 *
 */
#include "sndserv.h"
#include "mixer.h"

#include <stdatomic.h>
#include <sys/socket.h>

#define COMMAND_CAPACITY 256

struct data {
  struct pw_main_loop *loop;
  struct pw_stream *stream;
  struct spa_source *control_source;
  struct spa_source *notify_source;
  struct sound_message commands[COMMAND_CAPACITY];
  atomic_uint command_read;
  atomic_uint command_write;
  atomic_uint finished[SOUND_CHANNELS];
  uint32_t reported[SOUND_CHANNELS];
  bool status_pending;
  bool failed;
};

static bool
queue_full(struct data *data)
{
  unsigned int written = atomic_load_explicit(&data->command_write, memory_order_relaxed);
  unsigned int read = atomic_load_explicit(&data->command_read, memory_order_acquire);
  return written - read == COMMAND_CAPACITY;
}

static void
update_control_events(struct data *data)
{
  uint32_t events = SPA_IO_ERR | SPA_IO_HUP;
  if (!queue_full(data)) {
    events |= SPA_IO_IN;
  }
  if (data->status_pending) {
    events |= SPA_IO_OUT;
  }
  pw_loop_update_io(pw_main_loop_get_loop(data->loop), data->control_source, events);
}

static void
report_finished(struct data *data)
{
  data->status_pending = false;
  for (unsigned int slot = 0; slot < SOUND_CHANNELS; slot++) {
    uint32_t handle = atomic_load_explicit(&data->finished[slot], memory_order_acquire);
    if (handle == 0 || handle == data->reported[slot]) {
      continue;
    }
    struct sound_message message = {
      .version = SOUND_PROTOCOL_VERSION,
      .type = SOUND_FINISHED,
      .slot = slot,
      .handle = handle,
    };
    ssize_t result;
    do {
      result = send(STDIN_FILENO, &message, sizeof(message), MSG_DONTWAIT | MSG_NOSIGNAL);
    } while (result < 0 && errno == EINTR);
    if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      data->status_pending = true;
      break;
    }
    if (result != sizeof(message)) {
      pw_main_loop_quit(data->loop);
      return;
    }
    data->reported[slot] = handle;
  }
  update_control_events(data);
}

static void
on_notify(void *userdata, uint64_t count)
{
  (void)count;
  report_finished(userdata);
}

static void
on_control(void *userdata, int fd, uint32_t mask)
{
  struct data *data = userdata;
  if (mask & (SPA_IO_HUP | SPA_IO_ERR)) {
    pw_main_loop_quit(data->loop);
    return;
  }
  if (mask & SPA_IO_OUT) {
    report_finished(data);
  }
  while ((mask & SPA_IO_IN) && !queue_full(data)) {
    struct sound_message message;
    ssize_t length = recv(fd, &message, sizeof(message), MSG_DONTWAIT | MSG_TRUNC);
    if (length < 0 && errno == EINTR) {
      continue;
    }
    if (length < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      break;
    }
    if (length != sizeof(message) || !mixer_valid_command(&message)) {
      if (length != 0) {
        fprintf(stderr, "Invalid sound command or connection failure\n");
        data->failed = true;
      }
      pw_main_loop_quit(data->loop);
      return;
    }
    unsigned int written = atomic_load_explicit(&data->command_write, memory_order_relaxed);
    data->commands[written % COMMAND_CAPACITY] = message;
    atomic_store_explicit(&data->command_write, written + 1, memory_order_release);
  }
  update_control_events(data);
}

/* Process audio buffer */
static void
on_process(void *userdata)
{
  struct data *data = userdata;
  unsigned int read = atomic_load_explicit(&data->command_read, memory_order_relaxed);
  unsigned int written = atomic_load_explicit(&data->command_write, memory_order_acquire);
  bool notify = read != written;

  /* Apply a bounded batch of commands on the same thread that mixes audio */
  while (read != written) {
    mixer_command(&data->commands[read % COMMAND_CAPACITY]);
    read++;
  }
  atomic_store_explicit(&data->command_read, read, memory_order_release);

  struct pw_buffer *buffer = pw_stream_dequeue_buffer(data->stream);
  if (buffer) {
    struct spa_buffer *spa_buffer = buffer->buffer;
    buffer->size = 0;
    if (spa_buffer->n_datas > 0) {
      struct spa_data *plane = &spa_buffer->datas[0];
      if (plane->chunk) {
        plane->chunk->offset = 0;
        plane->chunk->stride = sizeof(int16_t) * 2;
        plane->chunk->size = 0;
        if (plane->data) {
          uint32_t frames = plane->maxsize / plane->chunk->stride;
          if (buffer->requested && buffer->requested < frames) {
            frames = (uint32_t)buffer->requested;
          }
          mixer_render(plane->data, frames);
          plane->chunk->size = frames * plane->chunk->stride;
          buffer->size = frames;
        }
      }
    }
    pw_stream_queue_buffer(data->stream, buffer);
  }

  for (unsigned int slot = 0; slot < SOUND_CHANNELS; slot++) {
    uint32_t handle = mixer_finished(slot);
    if (handle && handle != atomic_load_explicit(&data->finished[slot], memory_order_relaxed)) {
      atomic_store_explicit(&data->finished[slot], handle, memory_order_release);
      notify = true;
    }
  }
  if (notify) {
    pw_loop_signal_event(pw_main_loop_get_loop(data->loop), data->notify_source);
  }
}

static void
on_state_changed(void *userdata, enum pw_stream_state old, enum pw_stream_state state, const char *error)
{
  (void)old;
  struct data *data = userdata;
  if (state == PW_STREAM_STATE_ERROR) {
    fprintf(stderr, "Sound stream failed: %s\n", error ? error : "unknown error");
    data->failed = true;
    pw_main_loop_quit(data->loop);
  }
}

static const struct pw_stream_events stream_events = {
  PW_VERSION_STREAM_EVENTS,
  .state_changed = on_state_changed,
  .process = on_process,
};

static void
do_quit(void *userdata, int signal_number)
{
  (void)signal_number;
  struct data *data = userdata;
  pw_main_loop_quit(data->loop);
}

int
main(int argc, char *argv[])
{
  struct data data = { 0 };
  int result = EXIT_FAILURE;
  int socket_type;
  socklen_t socket_type_size = sizeof(socket_type);
  if (argc != 2 || getsockopt(STDIN_FILENO, SOL_SOCKET, SO_TYPE, &socket_type, &socket_type_size) < 0 ||
      socket_type != SOCK_SEQPACKET) {
    fprintf(stderr, "Start sndserver through DOOM with a WAD path and a sound control socket\n");
    return EXIT_FAILURE;
  }

  /* Load samples and initialize the mixer before starting audio callbacks */
  grabdata(argv[1]);
  mixer_init();
  atomic_init(&data.command_read, 0);
  atomic_init(&data.command_write, 0);
  for (unsigned int slot = 0; slot < SOUND_CHANNELS; slot++) {
    atomic_init(&data.finished[slot], 0);
  }
  if (!atomic_is_lock_free(&data.command_read) || !atomic_is_lock_free(&data.finished[0])) {
    fprintf(stderr, "Sound processing requires lock-free integer atomics\n");
    freedata();
    return EXIT_FAILURE;
  }

  /* Initialize PipeWire */
  pw_init(NULL, NULL);
  data.loop = pw_main_loop_new(NULL);
  if (!data.loop) {
    goto exit;
  }
  struct pw_loop *loop = pw_main_loop_get_loop(data.loop);
  if (!pw_loop_add_signal(loop, SIGINT, do_quit, &data) || !pw_loop_add_signal(loop, SIGTERM, do_quit, &data)) {
    goto exit;
  }
  data.notify_source = pw_loop_add_event(loop, on_notify, &data);
  data.control_source =
      pw_loop_add_io(loop, STDIN_FILENO, SPA_IO_IN | SPA_IO_HUP | SPA_IO_ERR, false, on_control, &data);
  if (!data.notify_source || !data.control_source) {
    goto exit;
  }

  const char *target = getenv("DOOM_AUDIO_TARGET");
  if (!target) {
    target = "AudioDevice0Output0";
  }
  struct pw_properties *props = pw_properties_new(PW_KEY_MEDIA_TYPE,
                                                  "Audio",
                                                  PW_KEY_MEDIA_CATEGORY,
                                                  "Playback",
                                                  PW_KEY_MEDIA_ROLE,
                                                  "Game",
                                                  PW_KEY_TARGET_OBJECT,
                                                  target,
                                                  NULL);
  if (!props) {
    goto exit;
  }
  data.stream = pw_stream_new_simple(loop, "DOOM sound server", props, &stream_events, &data);
  if (!data.stream) {
    goto exit;
  }

  uint8_t buffer[1024];
  struct spa_pod_builder builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  const struct spa_pod *params[1];
  struct spa_audio_info_raw raw = SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_S16_LE,
                                                          .channels = 2,
                                                          .rate = SOUND_RATE,
                                                          .position = { SPA_AUDIO_CHANNEL_FL, SPA_AUDIO_CHANNEL_FR });
  params[0] = spa_format_audio_raw_build(&builder, SPA_PARAM_EnumFormat, &raw);
  if (pw_stream_connect(data.stream,
                        PW_DIRECTION_OUTPUT,
                        PW_ID_ANY,
                        PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS,
                        params,
                        SPA_N_ELEMENTS(params)) < 0) {
    goto exit;
  }
  if (pw_main_loop_run(data.loop) >= 0 && !data.failed) {
    result = EXIT_SUCCESS;
  }

exit:
  /* Stop callbacks before releasing their queues and sample data */
  if (data.stream) {
    pw_stream_destroy(data.stream);
  }
  if (data.loop) {
    if (data.control_source) {
      pw_loop_destroy_source(pw_main_loop_get_loop(data.loop), data.control_source);
    }
    if (data.notify_source) {
      pw_loop_destroy_source(pw_main_loop_get_loop(data.loop), data.notify_source);
    }
    pw_main_loop_destroy(data.loop);
  }
  pw_deinit();
  freedata();
  return result;
}
