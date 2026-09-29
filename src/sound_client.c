#include "sound_client.h"
#include "sound_protocol.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

static int sound_socket = -1;
static pid_t sound_pid = -1;
static uint32_t handles[SOUND_CHANNELS];
static uint64_t started[SOUND_CHANNELS];
static uint64_t sequence;
static uint32_t next_handle;

static void
disconnect_sound(void)
{
  if (sound_socket >= 0) {
    close(sound_socket);
    sound_socket = -1;
  }
  memset(handles, 0, sizeof(handles));
}

void
sound_client_shutdown(void)
{
  disconnect_sound();
  if (sound_pid > 0) {
    kill(sound_pid, SIGTERM);
    while (waitpid(sound_pid, NULL, 0) < 0 && errno == EINTR) { }
    sound_pid = -1;
  }
}

int
sound_client_init(const char *program, const char *wad)
{
  int sockets[2];
  posix_spawn_file_actions_t actions;
  char *args[] = { (char *)program, (char *)wad, NULL };

  sound_client_shutdown();
  if (!program || !wad || socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockets) < 0) {
    return 0;
  }

  int result = posix_spawn_file_actions_init(&actions);
  if (result == 0) {
    result = posix_spawn_file_actions_addclose(&actions, sockets[0]);
    if (result == 0) {
      result = posix_spawn_file_actions_adddup2(&actions, sockets[1], STDIN_FILENO);
    }
    if (result == 0 && sockets[1] != STDIN_FILENO) {
      result = posix_spawn_file_actions_addclose(&actions, sockets[1]);
    }
    if (result == 0) {
      result = posix_spawn(&sound_pid, program, &actions, NULL, args, environ);
    }
    posix_spawn_file_actions_destroy(&actions);
  }
  close(sockets[1]);
  if (result != 0) {
    close(sockets[0]);
    sound_pid = -1;
    fprintf(stderr, "Could not start sound server: %s\n", strerror(result));
    return 0;
  }
  sound_socket = sockets[0];
  return 1;
}

static void
receive_status(void)
{
  struct sound_message message;
  while (sound_socket >= 0) {
    ssize_t length = recv(sound_socket, &message, sizeof(message), MSG_DONTWAIT | MSG_TRUNC);
    if (length < 0 && errno == EINTR) {
      continue;
    }
    if (length < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      return;
    }
    if (length != sizeof(message) || message.version != SOUND_PROTOCOL_VERSION || message.type != SOUND_FINISHED ||
        message.slot >= SOUND_CHANNELS) {
      disconnect_sound();
      return;
    }
    if (handles[message.slot] == message.handle) {
      handles[message.slot] = 0;
    }
  }
}

static int
find_handle(int handle)
{
  for (int slot = 0; slot < SOUND_CHANNELS; slot++) {
    if (handle > 0 && handles[slot] == (uint32_t)handle) {
      return slot;
    }
  }
  return -1;
}

static int
send_command(struct sound_message *message)
{
  if (sound_socket < 0) {
    return 0;
  }
  message->version = SOUND_PROTOCOL_VERSION;
  ssize_t result;
  do {
    result = send(sound_socket, message, sizeof(*message), MSG_DONTWAIT | MSG_NOSIGNAL);
  } while (result < 0 && errno == EINTR);
  if (result != sizeof(*message)) {
    fprintf(stderr, "Sound server connection failed\n");
    disconnect_sound();
    return 0;
  }
  return 1;
}

int
sound_client_start(int id, int volume, int separation, int pitch)
{
  receive_status();
  if (sound_socket < 0) {
    return -1;
  }
  int slot = 0;
  for (int i = 0; i < SOUND_CHANNELS; i++) {
    if (handles[i] == 0) {
      slot = i;
      break;
    }
    if (started[i] < started[slot]) {
      slot = i;
    }
  }
  do {
    next_handle = next_handle == INT_MAX ? 1 : next_handle + 1;
  } while (find_handle((int)next_handle) >= 0);

  struct sound_message message = {
    .type = SOUND_PLAY,
    .slot = slot,
    .handle = next_handle,
    .id = id,
    .volume = volume,
    .separation = separation,
    .pitch = pitch,
  };
  if (!send_command(&message)) {
    return -1;
  }
  handles[slot] = next_handle;
  started[slot] = ++sequence;
  return (int)next_handle;
}

void
sound_client_stop(int handle)
{
  receive_status();
  int slot = find_handle(handle);
  if (slot < 0) {
    return;
  }
  struct sound_message message = { .type = SOUND_STOP, .slot = slot, .handle = handle };
  send_command(&message);
  handles[slot] = 0;
}

int
sound_client_playing(int handle)
{
  receive_status();
  return find_handle(handle) >= 0;
}

void
sound_client_update(int handle, int volume, int separation, int pitch)
{
  receive_status();
  int slot = find_handle(handle);
  if (slot < 0) {
    return;
  }
  struct sound_message message = {
    .type = SOUND_UPDATE,
    .slot = slot,
    .handle = handle,
    .volume = volume,
    .separation = separation,
    .pitch = pitch,
  };
  send_command(&message);
}
