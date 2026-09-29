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
#include <libwebsockets.h>

#include "common.h"
#include "controls.h"
#include "./linuxdoom/i_main.h"

#define WS_PORT 9000

/* WebSocket callback function */
static int
ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  (void)user;
  switch (reason) {
  case LWS_CALLBACK_ESTABLISHED:
    syslog(LOG_INFO, "WebSocket connection established");
    controls_reset();
    break;

  case LWS_CALLBACK_RECEIVE:
    if (!lws_frame_is_binary(wsi) || !lws_is_first_fragment(wsi) || !lws_is_final_fragment(wsi) ||
        lws_remaining_packet_payload(wsi) != 0) {
      syslog(LOG_WARNING, "Ignoring incomplete or non-binary input message");
      break;
    }
    if (!controls_receive(in, len)) {
      syslog(LOG_WARNING, "Ignoring invalid input message");
    }
    break;

  case LWS_CALLBACK_CLOSED:
    syslog(LOG_INFO, "WebSocket connection closed");
    controls_reset();
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
  info.iface = "lo";
  info.options = LWS_SERVER_OPTION_DISABLE_IPV6;
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
  /* Detach the thread on termination */
  pthread_detach(ws_thread);
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
