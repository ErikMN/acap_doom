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

#include <syslog.h>
#include <sys/mman.h>

/* Axoverlay */
#include <axoverlay.h>
/* CGLM */
#include <cglm/cglm.h>
/* OpenGL ES */
#include <GLES2/gl2.h>

#define OVL_WIDTH axoverlay_get_max_resolution_width(AXOVERLAY_DEFAULT_CAMERA_ID, NULL)
#define OVL_HEIGHT axoverlay_get_max_resolution_height(AXOVERLAY_DEFAULT_CAMERA_ID, NULL)
#define RENDER_T 33

#define BYTES_PER_PIXEL 4
#define BUFFER_SIZE (SCREENWIDTH * SCREENHEIGHT * BYTES_PER_PIXEL)

#define FPS_PRINT 0

static GMainLoop *main_loop = NULL;
GLuint buffer_texture = 0;
uint8_t *rgba_buffer = NULL;
static int overlay_id = -1;

static const GLchar vShaderStr[] = "#version 320 es\n"
                                   "in vec2 in_texcoor;\n"
                                   "in vec4 vPosition;\n"
                                   "uniform mat4 transform;\n"
                                   "out vec2 vTexcoor;\n"
                                   "void main() \n"
                                   "{ \n"
                                   "  gl_Position = transform * vPosition;\n"
                                   "  vTexcoor = in_texcoor;\n"
                                   "} \n";

static const GLchar fShaderStr[] = "#version 320 es\n"
                                   "precision mediump float;\n"
                                   "uniform sampler2D in_sampler;\n"
                                   "in vec2 vTexcoor;\n"
                                   "out vec4 fragColor; \n"
                                   "void main()\n"
                                   "{\n"
                                   "  fragColor = texture(in_sampler, vTexcoor);\n"
                                   "}\n";

struct Shader {
  GLuint program;
  GLuint vertex;
  GLuint fragment;
};

static struct Shader shader;
static mat4 projection;

static bool
check_gl_error(const char *context)
{
  bool had_error = false;
  GLenum err = glGetError();

  while (err != GL_NO_ERROR) {
    had_error = true;
    const char *msg;

    switch (err) {
    case GL_INVALID_OPERATION:
      msg = "GL_INVALID_OPERATION";
      break;
    case GL_INVALID_ENUM:
      msg = "GL_INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      msg = "GL_INVALID_VALUE";
      break;
    case GL_OUT_OF_MEMORY:
      msg = "GL_OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      msg = "GL_INVALID_FRAMEBUFFER_OPERATION";
      break;
    default:
      msg = "UNKNOWN_ERROR";
      break;
    }
    syslog(LOG_ERR, "%s: %s", context, msg);
    err = glGetError();
  }

  return had_error;
}

static int
updateDOOMTexture(void)
{
  if (!buffer_texture || !glIsTexture(buffer_texture)) {
    syslog(LOG_WARNING, "Generating new texture");
    glGenTextures(1, &buffer_texture);
    if (buffer_texture == 0) {
      syslog(LOG_ERR, "Failed to generate texture");
      return -1;
    }
    glBindTexture(GL_TEXTURE_2D, buffer_texture);
    check_gl_error("updateDOOMTexture: glBindTexture");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer);
    check_gl_error("updateDOOMTexture: glTexImage2D");
  }
  glBindTexture(GL_TEXTURE_2D, buffer_texture);
  check_gl_error("updateDOOMTexture: glBindTexture");

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); /* GL_LINEAR */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); /* GL_LINEAR */
  check_gl_error("updateDOOMTexture: glTexParameteri");

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREENWIDTH, SCREENHEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, rgba_buffer);
  check_gl_error("updateDOOMTexture: glTexSubImage2D");

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
    return FALSE;
  }
  /* We have successfully synced a frame */
  axoverlay_redraw(&error);
  if (error) {
    syslog(LOG_ERR, "Unable to redraw");
    g_error_free(error);
  }

  return TRUE;
}

static GLuint
load_shader(GLenum type, const GLchar *shader_src)
{
  GLuint shader;
  GLint is_compiled;

  /* Create the shader object */
  shader = glCreateShader(type);
  check_gl_error("load_shader: glCreateShader");
  if (shader == 0) {
    return 0;
  }

  /* Load the shader source */
  glShaderSource(shader, 1, &shader_src, NULL);
  check_gl_error("load_shader: glShaderSource");

  /* Compile the shader */
  glCompileShader(shader);
  check_gl_error("load_shader: glCompileShader");

  /* Check the compile status */
  glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
  check_gl_error("load_shader: glGetShaderiv: GL_COMPILE_STATUS");

  if (!is_compiled) {
    GLint info_log_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    check_gl_error("load_shader: glGetShaderiv: GL_INFO_LOG_LENGTH");

    if (info_log_length > 1) {
      char *info_log = malloc(info_log_length);
      if (info_log) {
        glGetShaderInfoLog(shader, info_log_length, NULL, info_log);
        syslog(LOG_ERR, "Error compiling shader:\n%s", info_log);
        free(info_log);
      }
    }
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

static int
init_shaders(void)
{
  shader.vertex = load_shader(GL_VERTEX_SHADER, vShaderStr);
  check_gl_error("init_shaders: load_shader (vertex)");
  if (shader.vertex == 0) {
    syslog(LOG_ERR, "Could not load vertex shader");
    return -1;
  }

  shader.fragment = load_shader(GL_FRAGMENT_SHADER, fShaderStr);
  check_gl_error("init_shaders: load_shader (fragment)");
  if (shader.fragment == 0) {
    syslog(LOG_ERR, "Could not load fragment shader");
    return -1;
  }

  shader.program = glCreateProgram();
  check_gl_error("init_shaders: glCreateProgram");
  if (shader.program == 0) {
    syslog(LOG_ERR, "Could not create shader program");
    return -1;
  }

  glAttachShader(shader.program, shader.vertex);
  check_gl_error("init_shaders: glAttachShader (vertex)");

  glAttachShader(shader.program, shader.fragment);
  check_gl_error("init_shaders: glAttachShader (fragment)");

  glLinkProgram(shader.program);
  check_gl_error("init_shaders: glLinkProgram");

  /* Always detach shaders after a successful link */
  glDetachShader(shader.program, shader.vertex);
  glDetachShader(shader.program, shader.fragment);
  check_gl_error("init_shaders: glDetachShader");

  return 0;
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
  check_gl_error("render_triangle: glDepthFunc");

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  check_gl_error("render_triangle: glClear");

  glUseProgram(shader.program);
  check_gl_error("render_triangle: glUseProgram");

  glActiveTexture(GL_TEXTURE1);

  glBindTexture(GL_TEXTURE_2D, buffer_texture);
  check_gl_error("render_triangle: glBindTexture");

  GLint locSampler = glGetUniformLocation(shader.program, "in_sampler");
  glUniform1i(locSampler, 1);

  mat4 transform;
  glm_mat4_identity(transform);

  vec3 scale_vec = { 9.0f, 9.0f, 1.0f };
  glm_scale(transform, scale_vec);

  glm_mat4_mul(projection, transform, transform);

  GLint locTransform = glGetUniformLocation(shader.program, "transform");
  glUniformMatrix4fv(locTransform, 1, GL_FALSE, (const GLfloat *)transform);

  GLint locUVS = glGetAttribLocation(shader.program, "in_texcoor");
  glEnableVertexAttribArray(locUVS);
  glVertexAttribPointer(locUVS, 2, GL_FLOAT, GL_FALSE, 0, &uvs[0]);

  GLint locVPosition = glGetAttribLocation(shader.program, "vPosition");
  glEnableVertexAttribArray(locVPosition);
  glVertexAttribPointer(locVPosition, 4, GL_FLOAT, GL_FALSE, 0, &quad_vertex_buffer_data[0]);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#if FPS_PRINT
  printf("fps: %f\n", 1000.0 / duration);
#endif
}

static void
init_render(void)
{
  /* Set BG color */
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  check_gl_error("init_render: glClearColor");

  glClear(GL_COLOR_BUFFER_BIT);
  check_gl_error("init_render: glClear");

  glDisable(GL_DEPTH_TEST);
  check_gl_error("init_render: glDisable");
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
  glDeleteShader(shader.vertex);
  glDeleteShader(shader.fragment);
  glDeleteProgram(shader.program);
}

static void
set_ortho(vec2 clipping, vec2 resolution, mat4 dest)
{
  float f = clipping[0];   /* Far plane */
  float n = clipping[1];   /* Near plane */
  float r = resolution[0]; /* Right */
  float t = resolution[1]; /* Top */
  glm_ortho(-r, r, -t, t, -n, f, dest);
}

static void
initialize_projection(void)
{
  vec2 clipping = { 1.0f, 0.0f };
  vec2 resolution = { 16.0f, 9.0f };
  set_ortho(clipping, resolution, projection);
}

/* Game thread */
static void *
doom_run(void *arg)
{
  (void)arg;
  D_DoomMain();

  return NULL;
}

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
  /* Propagate input arguments to global variables */
  myargc = argc;
  myargv = argv;

  print_debug("\n*** Development build\n");
  print_debug("Build: %s %s\n", __DATE__, __TIME__);

  struct axoverlay_settings ovl_settings;
  struct axoverlay_overlay_data ovl_data;
  GError *error = NULL;
  int ret = EXIT_FAILURE;

  /* Allocate aligned memory for buffers */
  rgba_buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (rgba_buffer == MAP_FAILED) {
    syslog(LOG_ERR, "mmap failed for buffer");
    goto exit;
  }

  /* Create the main loop */
  main_loop = g_main_loop_new(NULL, FALSE);

  /* Setup signal handling */
  init_signals();

  /* Init axoverlay settings */
  axoverlay_init_axoverlay_settings(&ovl_settings);
  ovl_settings.render_callback = render_overlay_cb;
  ovl_settings.adjustment_callback = NULL;
  ovl_settings.select_callback = NULL;
  ovl_settings.backend = AXOVERLAY_OPENGLES_BACKEND;

  /* Init axoverlay */
  axoverlay_init(&ovl_settings, &error);
  if (error != NULL) {
    syslog(LOG_ERR, "Failed to initialize axoverlay: %s", error->message);
    g_error_free(error);
    goto exit;
  }

  /* Init shaders */
  if (init_shaders() != 0) {
    goto exit;
  }
  /* Init projection matrix */
  initialize_projection();

  /* Init axoverlay data */
  axoverlay_init_overlay_data(&ovl_data);
  ovl_data.postype = AXOVERLAY_TOP_RIGHT;
  ovl_data.anchor_point = AXOVERLAY_ANCHOR_CENTER;
  ovl_data.width = OVL_WIDTH;
  ovl_data.height = OVL_HEIGHT;
  ovl_data.x = 0.0;
  ovl_data.y = 0.0;
  ovl_data.scale_to_stream = TRUE;
  ovl_data.colorspace = AXOVERLAY_COLORSPACE_ARGB32;

  /* Create overlay */
  overlay_id = axoverlay_create_overlay(&ovl_data, NULL, &error);
  if (error) {
    syslog(LOG_ERR, "Failed to create first overlay: %s", error->message);
    g_error_free(error);
    goto exit;
  }

  /* Start game thread */
  print_debug("\n*** START DOOM LOOP\n");
  pthread_t doom_thread;
  if (pthread_create(&doom_thread, NULL, doom_run, NULL) != 0) {
    syslog(LOG_ERR, "Failed to create Doom thread");
    goto exit;
  }

  /* Start render loop */
  g_timeout_add(RENDER_T, (GSourceFunc)render_trigger_redraw, NULL);

  /* Start GMainLoop */
  print_debug("\n*** START GMAIN LOOP\n");
  g_main_loop_run(main_loop);

  ret = EXIT_SUCCESS;

exit:
  /* Clean up and exit */
  print_debug("\n*** STOP APP\n");
  axoverlay_destroy_overlay(overlay_id, &error);
  if (error != NULL) {
    syslog(LOG_ERR, "Failed to destroy first overlay: %s", error->message);
    g_error_free(error);
  }
  cleanup_shaders();
  axoverlay_cleanup();
  if (rgba_buffer != MAP_FAILED) {
    if (rgba_buffer != NULL) {
      if (munmap(rgba_buffer, BUFFER_SIZE) == -1) {
        syslog(LOG_ERR, "munmap failed for buffer");
      }
    }
    rgba_buffer = MAP_FAILED;
  }
  glDeleteTextures(2, &buffer_texture);
  g_main_loop_unref(main_loop);

  syslog(LOG_INFO, "Quitting real main");

  return ret;
}
