#pragma once
#include "box3d/box3d.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Status: 0 invalid/stale argument, 1 valid, 2 busy (including reentrant
// callbacks), 3 sticky physics failure. Clearing API errors cannot heal a world.
enum { GPU_DIAGNOSTIC_INVALID = 0, GPU_DIAGNOSTIC_OK = 1,
       GPU_DIAGNOSTIC_BUSY = 2, GPU_DIAGNOSTIC_FAILED = 3 };

// GPU durations in milliseconds. The 23 bits follow the pinned b3Profile field
// order. Unmeasured fields are NaN. Timestamp reads poll without waiting; the
// returned timestamp_step may precede physics_step or be zero (no timestamps).
typedef struct GpuNativeProfile {
    uint64_t physics_step;
    uint64_t timestamp_step;
    uint32_t valid;
    uint32_t measured_fields;
    float values[23];
} GpuNativeProfile;

// Actual primary owned physics buffers only: excludes staging, cache copies,
// pipelines, renderer and driver allocations. Capacities reserve internal slots
// and are distinct from public live/peak occupancy counts.
typedef struct GpuNativeAllocation {
    uint64_t primary_buffer_bytes;
    uint32_t valid;
    uint32_t body_capacity;
    uint32_t shape_capacity;
    uint32_t joint_capacity;
    uint32_t contact_capacity;
    uint32_t physics_invalid;
} GpuNativeAllocation;

typedef struct GpuNativeShapeBounds {
    b3ShapeId shape;
    b3BodyId body;
    b3AABB bounds;
} GpuNativeShapeBounds;
typedef void GpuNativeBoundsVisitor(const GpuNativeShapeBounds* bounds, void* context);

// Counters explicitly synchronize contact records and may block. CPU-only
// fields and unavailable island labels use -1; see the published API contract.
uint32_t gpu_b3_world_native_counters(b3WorldId world, b3Counters* counters);
void gpu_b3_world_native_profile(b3WorldId world, GpuNativeProfile* profile);
// Peaks: public body/proxy occupancy at positive step boundaries, and supported
// non-sensor contact roots at occupied-list collection before CCD correction.
uint32_t gpu_b3_world_native_max_capacity(b3WorldId world, b3Capacity* capacity);
void gpu_b3_world_native_allocation(b3WorldId world, GpuNativeAllocation* allocation);
// One synchronized ownership snapshot; visitor executes after unlocking. Bounds
// include speculative padding and merge compound children under their public ID.
uint32_t gpu_b3_world_native_visit_shape_bounds(b3WorldId world, uint32_t body_type,
                                              GpuNativeBoundsVisitor* visitor, void* context);
// Cheap live public topology; no GPU wait or contact/body download.
void gpu_b3_world_counts(b3WorldId world, int* bodies, int* shapes, int* joints);

#ifdef __cplusplus
}
#endif
