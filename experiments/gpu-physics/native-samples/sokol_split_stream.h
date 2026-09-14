#pragma once
#include "sokol_gfx.h"
#ifdef __cplusplus
extern "C" {
#endif
void split_stream_view(int view);
void split_update_buffer(sg_buffer buffer, const sg_range* data);
void split_apply_bindings(const sg_bindings* bindings);
void split_destroy_buffer(sg_buffer buffer);
void split_destroy_view(sg_view view);
void split_render_viewport(int x, int width);
#ifdef __cplusplus
}
#endif
#ifdef SPLIT_STREAM_REMAP
#define sg_update_buffer split_update_buffer
#define sg_apply_bindings split_apply_bindings
#define sg_destroy_buffer split_destroy_buffer
#define sg_destroy_view split_destroy_view
#endif
