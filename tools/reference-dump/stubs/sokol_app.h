#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SAPP_KEYCODE_INVALID = 0,
  SAPP_KEYCODE_A = 1, SAPP_KEYCODE_B = 2, SAPP_KEYCODE_D = 3,
  SAPP_KEYCODE_F = 4, SAPP_KEYCODE_L = 5, SAPP_KEYCODE_M = 6,
  SAPP_KEYCODE_O = 7, SAPP_KEYCODE_P = 8, SAPP_KEYCODE_Q = 9,
  SAPP_KEYCODE_R = 10, SAPP_KEYCODE_S = 11, SAPP_KEYCODE_T = 12,
  SAPP_KEYCODE_V = 13, SAPP_KEYCODE_W = 14, SAPP_KEYCODE_SPACE = 15,
  SAPP_KEYCODE_ESCAPE = 16, SAPP_KEYCODE_TAB = 17,
  SAPP_KEYCODE_LEFT_SHIFT = 18, SAPP_KEYCODE_LEFT_BRACKET = 19,
  SAPP_KEYCODE_RIGHT_BRACKET = 20, SAPP_KEYCODE_PAUSE = 21,
  _SAPP_KEYCODE_NUM,
} sapp_keycode;

enum {
  SAPP_MODIFIER_SHIFT = 1,
  SAPP_MODIFIER_CTRL = 2,
  SAPP_MODIFIER_ALT = 4,
};

enum {
  SAPP_MOUSEBUTTON_LEFT = 1,
  SAPP_MOUSEBUTTON_RIGHT = 2,
  SAPP_MOUSEBUTTON_MIDDLE = 3,
};

typedef struct sapp_event { int dummy; } sapp_event;

typedef struct sapp_desc {
  void (*init_cb)(void);
  void (*frame_cb)(void);
  void (*event_cb)(const sapp_event*);
  void (*cleanup_cb)(void);
  struct { int major, minor; } gl;
  int width, height, sample_count;
  const char* window_title;
  int swap_interval;
  bool high_dpi;
  void (*logger)(const char*, uint32_t, uint32_t, const char*, uint32_t, const char*, void*);
  void* user_data;
} sapp_desc;

int sapp_width(void);
int sapp_height(void);
void sapp_quit(void);
double sapp_frame_duration(void);
uint64_t sapp_frame_count(void);
void sapp_lock_mouse(bool lock);
void sapp_request_quit(void);

#ifdef __cplusplus
}
#endif
