#include "sokol_preview_bounds.h"
#include "gfx/draw.h"
#include "box3d/math_functions.h"
#include "box3d/id.h"
#include <math.h>

static bool s_enabled, s_currentValid, s_completeValid, s_initialPending;
static uint32_t s_currentWorld, s_completeWorld, s_initialWorld;
static b3AABB s_current, s_complete;

void sample_preview_begin_frame(b3WorldId world, bool enabled)
{
    s_currentWorld = b3StoreWorldId(world);
    s_enabled = enabled;
    s_currentValid = false;
}

void sample_preview_end_frame(void)
{
    s_completeWorld = s_currentWorld;
    s_completeValid = s_enabled && s_currentValid;
    if (s_completeValid) s_complete = s_current;
    s_enabled = false;
}

void sample_preview_include_relative(b3Vec3 point, float radius)
{
    if (!s_enabled) return;
    b3Vec3 absolute = b3ToVec3(b3OffsetPos(GetDrawOrigin(), point));
    radius = fabsf(radius);
    if (!b3IsValidVec3(absolute) || !isfinite(radius)) return;
    b3Vec3 extent = {radius, radius, radius};
    b3AABB bounds = {b3Sub(absolute, extent), b3Add(absolute, extent)};
    if (!s_currentValid)
    {
        s_current = bounds;
        s_currentValid = true;
    }
    else
    {
        s_current.lowerBound = b3Min(s_current.lowerBound, bounds.lowerBound);
        s_current.upperBound = b3Max(s_current.upperBound, bounds.upperBound);
    }
}

void sample_preview_include_cube(b3Transform transform, b3Vec3 scale)
{
    if (!s_enabled) return;
    b3Vec3 half = b3MulSV(0.5f, scale);
    for (int i = 0; i < 8; ++i)
    {
        b3Vec3 local = {(i & 1) ? half.x : -half.x, (i & 2) ? half.y : -half.y, (i & 4) ? half.z : -half.z};
        sample_preview_include_relative(b3TransformPoint(transform, local), 0.0f);
    }
}

void sample_preview_include_sphere(b3Transform transform, float radius)
{
    sample_preview_include_relative(transform.p, radius);
}

void sample_preview_include_capsule(b3Transform transform, float halfLength, float radius)
{
    if (!s_enabled) return;
    sample_preview_include_relative(b3TransformPoint(transform, (b3Vec3){-halfLength, 0.0f, 0.0f}), radius);
    sample_preview_include_relative(b3TransformPoint(transform, (b3Vec3){halfLength, 0.0f, 0.0f}), radius);
}

bool sample_preview_get_bounds(b3WorldId world, b3AABB* bounds)
{
    if (!s_completeValid || s_completeWorld != b3StoreWorldId(world)) return false;
    *bounds = s_complete;
    return true;
}

void sample_preview_request_initial_frame(b3WorldId world)
{
    s_initialWorld = b3StoreWorldId(world);
    s_initialPending = true;
}

bool sample_preview_take_initial_frame(b3WorldId world, b3AABB* bounds)
{
    if (!s_initialPending || s_initialWorld != b3StoreWorldId(world) || !sample_preview_get_bounds(world, bounds)) return false;
    s_initialPending = false;
    return true;
}
