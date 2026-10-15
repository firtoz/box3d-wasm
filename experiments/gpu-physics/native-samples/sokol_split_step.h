#pragma once
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// Records only accepted direct Step submissions. World geometry is drawn
// independently for each pane, and Render submissions run independently too.
void split_step_begin(bool enabled);
void split_step_end(void);
void split_step_world_begin(void);
void split_step_world_end(void);
void split_step_record(void (*replay)(const void*), const void* data, size_t size);
void split_step_replay(void);
void split_step_ui_begin(int pane_width);
void split_step_ui_end(void);
int split_step_ui_width(void);
void split_step_shutdown(void);
#ifdef __cplusplus
}
#endif
