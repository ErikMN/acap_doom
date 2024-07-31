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
//	Main program, simply calls D_DoomMain high level loop.
//
// Modified 2023 to run on Axis devices.
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "m_argv.h"
#include "d_main.h"
#include "i_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <sys/mman.h>
#include <pthread.h>

/* Axoverlay */
#include <axoverlay.h>

/* CGLM */
#include <cglm/cglm.h>

/* OpenGL ES */
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>
#undef GL_GLEXT_PROTOTYPES

#define OVL_WIDTH axoverlay_get_max_resolution_width(AXOVERLAY_DEFAULT_CAMERA_ID, NULL)
#define OVL_HEIGHT axoverlay_get_max_resolution_height(AXOVERLAY_DEFAULT_CAMERA_ID, NULL)
#define RENDER_T 33

#define BYTES_PER_PIXEL 4
#define BUFFER_SIZE (SCREENWIDTH * SCREENHEIGHT * BYTES_PER_PIXEL)

#define FPS_PRINT 0

GLuint buffer_texture = 0;
uint8_t *rgba_buffer = NULL;
static int overlay_id1 = -1;

static const GLbyte vShaderStr[] = "#version 320 es\n"
                                   "in vec2 in_texcoor;\n"
                                   "in vec4 vPosition;\n"
                                   "uniform mat4 transform;\n"
                                   "out vec2 vTexcoor;\n"
                                   "void main() \n"
                                   "{ \n"
                                   "  vec4 pos = transform * vPosition;\n"
                                   "  vTexcoor = in_texcoor;\n"
                                   "  gl_Position = pos;\n"
                                   "} \n";

static const GLbyte fShaderStr[] = "#version 320 es\n"
                                   "precision mediump float;\n"
                                   "uniform sampler2D in_sampler;\n"
                                   "in vec2 vTexcoor;\n"
                                   "out vec4 fragColor; \n"
                                   ""
                                   "void main()\n"
                                   "{\n"
                                   "  fragColor = texture(in_sampler, vTexcoor);\n"
                                   "}\n";

struct Shader {
  GLuint shaderProgram;
  GLuint vertexShader;
  GLuint fragmentShader;
};

static struct Shader *shader = NULL;
static mat4 projection;

static bool
check_gl_error(const char *context)
{
  bool had_error = false;
  GLenum err = glGetError();
  char *msg;

  while (err != GL_NO_ERROR) {
    had_error = true;
    switch (err) {
    case GL_INVALID_OPERATION:
      msg = (char *)"glGetError() has reported an INVALID_OPERATION error";
      break;
    case GL_INVALID_ENUM:
      msg = (char *)"glGetError() has reported an INVALID_ENUM error";
      break;
    case GL_INVALID_VALUE:
      msg = (char *)"glGetError() has reported an INVALID_VALUE error";
      break;
    case GL_OUT_OF_MEMORY:
      msg = (char *)"glGetError() has reported an OUT_OF_MEMORY error";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      msg = (char *)"glGetError() has reported an INVALID_FRAMEBUFFER_OPERATION error";
      break;
    default:
      msg = (char *)"glGetError() has reported an UNKNOWN_ERROR";
      break;
    }
    syslog(LOG_INFO, "%s: %s", context, msg);
    err = glGetError();
  }
  return had_error;
}

static int
updateDOOMTexture(void)
{
  if (!buffer_texture || !glIsTexture(buffer_texture)) {
    syslog(LOG_WARNING, "This should only be written once per buffer");
    glGenTextures(1, &buffer_texture);
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer);
    if (glIsTexture(buffer_texture) == GL_FALSE) {
      syslog(LOG_ERR, "The target texture is invalid!");
      return -1;
    }
  }
  check_gl_error("updateDOOMTexture - glTexImage2D");
  glBindTexture(GL_TEXTURE_2D, buffer_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); /* GL_LINEAR */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); /* GL_LINEAR */
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREENWIDTH, SCREENHEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer);
  check_gl_error("updateDOOMTexture - glTexSubImage2D");

  return 0;
}

static gboolean
render_trigger_redraw(gpointer user_data)
{
  (void)user_data;
  // printf("*** render_trigger_redraw\n");
  GError *error = NULL;
  int ret = updateDOOMTexture();
  if (ret < 0) {
    syslog(LOG_ERR, "Unable to update DOOM framebuffer");
    return false;
  }
  /* We have successfully synced a frame */
  axoverlay_redraw(&error);
  if (error) {
    syslog(LOG_ERR, "Unable to redraw");
    g_error_free(error);
  }

  return TRUE;
}

static void
init_render(void)
{
  /* Set BG color */
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  check_gl_error("init_render - glClearColor");
  glClear(GL_COLOR_BUFFER_BIT);
  check_gl_error("init_render - glClear");
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable(GL_BLEND);
  check_gl_error("init_render - glEnable");
  glDisable(GL_DEPTH_TEST);
}

static GLuint
LoadShader(GLenum type, const char *shaderSrc)
{
  GLuint shader;
  GLint compiled;

  /* Create the shader object */
  shader = glCreateShader(type);
  check_gl_error("LoadShader - glCreateShader");
  if (shader == 0) {
    return 0;
  }

  /* Load the shader source */
  glShaderSource(shader, 1, &shaderSrc, NULL);
  check_gl_error("LoadShader - glShaderSource");

  /* Compile the shader */
  glCompileShader(shader);
  check_gl_error("LoadShader - glCompileShader");

  /* Check the compile status */
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  check_gl_error("LoadShader - glGetShaderiv");

  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1) {
      char *infoLog = (char *)malloc(sizeof(char) * infoLen);
      glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
      print_debug("Error compiling shader:\n%s\n", infoLog);
      free(infoLog);
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

static void
init_shaders(void)
{
  /* create shader program */
  shader = (struct Shader *)malloc(sizeof(struct Shader));
  if (!shader) {
    fprintf(stderr, "Failed to allocate memory for Shader\n");
    exit(EXIT_FAILURE);
  }

  shader->vertexShader = LoadShader(GL_VERTEX_SHADER, (const char *)vShaderStr);
  check_gl_error("render_overlay_cb - LoadShader (vertex)");

  shader->fragmentShader = LoadShader(GL_FRAGMENT_SHADER, (const char *)fShaderStr);
  check_gl_error("render_overlay_cb - LoadShader (fragment)");

  shader->shaderProgram = glCreateProgram();
  check_gl_error("Could not create shader program");
  if (shader->shaderProgram == 0) {
    free(shader);
    shader = NULL;
    return;
  }

  glAttachShader(shader->shaderProgram, shader->vertexShader);
  check_gl_error("render_overlay_cb - glAttachShader (vertex)");

  glAttachShader(shader->shaderProgram, shader->fragmentShader);
  check_gl_error("render_overlay_cb - glAttachShader (fragment)");

  glLinkProgram(shader->shaderProgram);
  check_gl_error("render_overlay_cb - glLinkProgram");

  /* Always detach shaders after a successful link*/
  glDetachShader(shader->shaderProgram, shader->vertexShader);
  glDetachShader(shader->shaderProgram, shader->fragmentShader);
}

static void
render_triangle(void)
{
#if FPS_PRINT
  static struct timespec start;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  double duration = (now.tv_sec - start.tv_sec) * 1000.0 + (now.tv_nsec - start.tv_nsec) / 1000000.0;
  start = now;
#endif

  static const float aspect_ratio = 4.0f / 3.0f;

  /* clang-format off */
  static const vec4 quad_vertex_buffer_data[] = {
    { -aspect_ratio, -1.0f, 0.0f, 1.0f },
    { -aspect_ratio,  1.0f, 0.0f, 1.0f },
    {  aspect_ratio,  1.0f, 0.0f, 1.0f },
    {  aspect_ratio, -1.0f, 0.0f, 1.0f }
  };

  static const vec2 uvs[] = {
    {  0.0f, 0.0f },
    {  0.0f, 1.0f },
    {  1.0f, 1.0f },
    {  1.0f, 0.0f }
  };
  /* clang-format on */

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
  check_gl_error("render_triangle - glDepthFunc");

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  check_gl_error("render_triangle - glClear");

  glUseProgram(shader->shaderProgram);
  check_gl_error("render_triangle - glUseProgram");

  glActiveTexture(GL_TEXTURE1);

  glBindTexture(GL_TEXTURE_2D, buffer_texture);
  GLint locSampler = glGetUniformLocation(shader->shaderProgram, "in_sampler");
  glUniform1i(locSampler, 1);

  mat4 transform;
  glm_mat4_identity(transform);

  vec3 scale_vec = { 9.0f, 9.0f, 1.0f };
  glm_scale(transform, scale_vec);

  glm_mat4_mul(projection, transform, transform);

  GLint locTransform = glGetUniformLocation(shader->shaderProgram, "transform");
  glUniformMatrix4fv(locTransform, 1, GL_FALSE, (const GLfloat *)transform);

  GLint locUVS = glGetAttribLocation(shader->shaderProgram, "in_texcoor");
  glEnableVertexAttribArray(locUVS);
  glVertexAttribPointer(locUVS, 2, GL_FLOAT, GL_FALSE, 0, &uvs[0]);

  GLint locVPosition = glGetAttribLocation(shader->shaderProgram, "vPosition");
  glEnableVertexAttribArray(locVPosition);
  glVertexAttribPointer(locVPosition, 4, GL_FLOAT, GL_FALSE, 0, &quad_vertex_buffer_data[0]);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#if FPS_PRINT
  printf("fps: %f\n", 1000.0 / duration);
#endif
}

static void
render_overlay_cb(gpointer render_context,
                  gint id,
                  struct axoverlay_stream_data *stream,
                  enum axoverlay_position_type postype,
                  gfloat overlay_x,
                  gfloat overlay_y,
                  gint overlay_width,
                  gint overlay_height,
                  gpointer user_data)
{
  (void)render_context;
  (void)id;
  (void)stream;
  (void)postype;
  (void)overlay_x;
  (void)overlay_y;
  (void)overlay_width;
  (void)overlay_height;
  (void)user_data;
  init_render();
  render_triangle();
}

static void
cleanup_shaders(void)
{
  glDeleteShader(shader->vertexShader);
  glDeleteShader(shader->fragmentShader);
  glDeleteProgram(shader->shaderProgram);
  free(shader);
}

static void
setOrtho(vec2 clipping, vec2 resolution, mat4 dest)
{
  float f = clipping[0];   /* Far plane */
  float n = clipping[1];   /* Near plane */
  float r = resolution[0]; /* Right */
  float t = resolution[1]; /* Top */
  glm_ortho(-r, r, -t, t, -n, f, dest);
}

static void
initialize_projection()
{
  vec2 clipping = { 1.0f, 0.0f };
  vec2 resolution = { 16.0f, 9.0f };
  setOrtho(clipping, resolution, projection);
}

/* Game thread */
void *
doom_run(void *arg)
{
  (void)arg;
  D_DoomMain();
  return NULL;
}

static GMainLoop *main_loop = NULL;

static void
handle_sigterm(int signo)
{
  (void)signo;
  g_main_loop_quit(main_loop);
}

static void
init_signals(void)
{
  struct sigaction sa;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = handle_sigterm;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
}

int
real_main(int argc, char **argv)
{
  myargc = argc;
  myargv = argv;

  print_debug("\n*** Development build\n");
  print_debug("Build: %s %s\n", __DATE__, __TIME__);

  struct axoverlay_settings settings;
  struct axoverlay_overlay_data data1;
  GError *error = NULL;

  /* Allocate aligned memory for buffers */
  rgba_buffer = (uint8_t *)mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (rgba_buffer == MAP_FAILED) {
    perror("mmap failed");
    return 1;
  }

  /* Create the main loop */
  main_loop = g_main_loop_new(NULL, FALSE);

  /* Setup signal handling */
  init_signals();

  /* Init axoverlay settings */
  axoverlay_init_axoverlay_settings(&settings);
  settings.render_callback = render_overlay_cb;
  settings.adjustment_callback = NULL;
  settings.select_callback = NULL;
  settings.backend = AXOVERLAY_OPENGLES_BACKEND;

  /* Init axoverlay */
  axoverlay_init(&settings, &error);
  if (error != NULL) {
    syslog(LOG_ERR, "Failed to initialize axoverlay: %s", error->message);
    g_error_free(error);
    return 1;
  }

  /* Init shaders */
  init_shaders();
  initialize_projection();

  /* Init axoverlay data */
  axoverlay_init_overlay_data(&data1);
  data1.postype = AXOVERLAY_TOP_RIGHT;
  data1.anchor_point = AXOVERLAY_ANCHOR_CENTER;
  data1.width = OVL_WIDTH;
  data1.height = OVL_HEIGHT;
  data1.x = 0.0;
  data1.y = 0.0;
  data1.scale_to_stream = TRUE;
  data1.colorspace = AXOVERLAY_COLORSPACE_ARGB32;

  /* Create overlay */
  overlay_id1 = axoverlay_create_overlay(&data1, NULL, &error);
  if (error) {
    syslog(LOG_ERR, "Failed to create first overlay: %s", error->message);
    g_error_free(error);
    return 1;
  }

  /* Start game thread */
  print_debug("\n*** START DOOM LOOP\n");
  pthread_t doom_thread;
  if (pthread_create(&doom_thread, NULL, doom_run, NULL) != 0) {
    perror("pthread_create");
    goto exit;
  }

  /* Start render loop */
  g_timeout_add(RENDER_T, (GSourceFunc)render_trigger_redraw, NULL);

  /* Start GMainLoop */
  print_debug("\n*** START GMAIN LOOP\n");
  g_main_loop_run(main_loop);

  /* Add cleanup code here */
  axoverlay_destroy_overlay(overlay_id1, &error);
  if (error != NULL) {
    syslog(LOG_ERR, "Failed to destroy first overlay: %s", error->message);
    g_error_free(error);
    return 0;
  }

exit:
  print_debug("\n*** STOP APP\n");
  cleanup_shaders();
  axoverlay_cleanup();
  if (rgba_buffer != NULL) {
    if (munmap(rgba_buffer, BUFFER_SIZE) == -1) {
      syslog(LOG_ERR, "munmap failed for buffer");
    } else {
      rgba_buffer = NULL;
    }
  }
  glDeleteTextures(2, &buffer_texture);
  g_main_loop_unref(main_loop);

  syslog(LOG_INFO, "Quitting real main");

  return 0;
}
