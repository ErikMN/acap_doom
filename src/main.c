#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdint.h>
#include <syslog.h>

#include <pthread.h>
#include <jansson.h>
#include <libwebsockets.h>

#include "common.h"
#include "controls.h"
#include "./linuxdoom/i_main.h"

#define WS_PORT 9000

#define MAX_JSON_SIZE 1024 /* Define a safe maximum size for JSON data */
#define MAX_KEY_PRESS_VALUE_LENGTH 255
#define RESPONSE_BUFFER_SIZE (LWS_PRE + MAX_KEY_PRESS_VALUE_LENGTH)

static char json_data[MAX_JSON_SIZE + 1] = { 0 };
static unsigned char buffer[RESPONSE_BUFFER_SIZE] = { 0 };

/* WebSocket callback function */
static int
ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  (void)user;
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED:
    syslog(LOG_INFO, "WebSocket connection established");
    break;

  case LWS_CALLBACK_RECEIVE:
    if (len > MAX_JSON_SIZE) {
      syslog(LOG_ERR, "Received data too large");
      return -1;
    }
    memcpy(json_data, in, len);
    json_data[len] = '\0';
    json_error_t error;
    json_t *root = json_loads(json_data, 0, &error);

    if (!root) {
      syslog(LOG_ERR, "JSON parsing error on line %d: %s", error.line, error.text);
      return -1;
    }

    json_t *keyPress = json_object_get(root, "keyPress");
    if (!json_is_string(keyPress)) {
      syslog(LOG_ERR, "Expected a string for keyPress");
      json_decref(root);
      return -1;
    }

    const char *keyPressValue = json_string_value(keyPress);
    size_t n = strlen(keyPressValue);
    if (n > MAX_KEY_PRESS_VALUE_LENGTH) {
      syslog(LOG_ERR, "keyPressValue too long");
      json_decref(root);
      return -1;
    }

    /* Set the game key state */
    set_key_state(keyPressValue);

    unsigned char *p = &buffer[LWS_PRE];
    memcpy(p, keyPressValue, n);

    /* Send the key press value back to the client */
    int write_result = lws_write(wsi, p, n, LWS_WRITE_TEXT);
    if (write_result < 0) {
      syslog(LOG_ERR, "Error writing to socket");
    } else if ((size_t)write_result < n) {
      syslog(LOG_WARNING, "Partial write");
    }
    json_decref(root);
    break;

  default:
    break;
  }

  return 0;
}

static void *
ws_run(void *arg)
{
  (void)arg;
  struct lws_context *context = NULL;
  struct lws_context_creation_info info;

  const struct lws_http_mount mount = { .mountpoint = "/ws" };
  static struct lws_protocols protocols[] = { {
                                                  .name = "ws",
                                                  .callback = ws_callback,
                                                  .per_session_data_size = 0,
                                                  .id = 0,
                                              },
                                              LWS_PROTOCOL_LIST_TERM };
  memset(&info, 0, sizeof(info));
  info.port = WS_PORT;
  info.protocols = protocols;
  info.mounts = &mount;
  info.gid = -1;
  info.uid = -1;

  /* Set log level to error and warning only */
  lws_set_log_level(LLL_ERR | LLL_WARN, NULL);

  context = lws_create_context(&info);

  if (!context) {
    syslog(LOG_ERR, "Failed to create libwebsocket context");
    return NULL;
  }
  PRINT_GREEN("WebSocket server started on port %d\n", info.port);

  while (1) {
    /* Non-blocking timeout: 0ms */
    lws_service(context, 5);
  }
  lws_context_destroy(context);

  return NULL;
}

static int
ws_setup(void)
{
  pthread_t ws_thread;
  /* Create websocket thread */
  syslog(LOG_INFO, "Start websocket thread");
  if (pthread_create(&ws_thread, NULL, ws_run, NULL) != 0) {
    syslog(LOG_ERR, "Failed to set up WebSocket. Terminating.");
    return -1;
  }

  return 0;
}

int
main(int argc, char **argv)
{
  int ret = 0;

  /* Open the syslog to report messages for the app */
  openlog(APP_NAME, LOG_PID | LOG_CONS, LOG_USER);

  /* Choose between { LOG_INFO, LOG_CRIT, LOG_WARN, LOG_ERR } */
  syslog(LOG_INFO, "Starting %s", APP_NAME);

  /* Setup websocket */
  if (ws_setup() != 0) {
    exit(EXIT_FAILURE);
  }

  /* Start the APP here */
  ret = real_main(argc, argv);

  syslog(LOG_INFO, "Terminating %s", APP_NAME);

  /* Close application logging to syslog */
  closelog();

  return ret;
}
