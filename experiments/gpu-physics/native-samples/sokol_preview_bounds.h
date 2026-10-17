#pragma once
#include "box3d/types.h"
#include <stdbool.h>

#define SAMPLE_PREVIEW_FRAME_PADDING 1.25f

#ifdef __cplusplus
extern "C" {
#endif
void sample_preview_begin_frame(b3WorldId world, bool enabled);
void sample_preview_end_frame(void);
void sample_preview_include_relative(b3Vec3 point, float radius);
void sample_preview_include_cube(b3Transform transform, b3Vec3 scale);
void sample_preview_include_sphere(b3Transform transform, float radius);
void sample_preview_include_capsule(b3Transform transform, float halfLength, float radius);
bool sample_preview_get_bounds(b3WorldId world, b3AABB* bounds);
void sample_preview_request_initial_frame(b3WorldId world);
bool sample_preview_take_initial_frame(b3WorldId world, b3AABB* bounds);
#ifdef __cplusplus
}
#endif
