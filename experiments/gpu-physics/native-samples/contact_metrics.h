#pragma once
#include "box3d/box3d.h"
#include <stdint.h>
#include <stddef.h>
// Keep layout identical to Rust api::WorldContactMetrics.
struct GpuContactMetrics {
 uint64_t snapshot_step, submitted_step, snapshot_topology, current_topology, snapshot_state, current_state;
 uint32_t known, current, capacity_loss, candidate_pairs, allocated_roots, allocated_manifold_slots, touching_roots, non_sensor_roots;
};
static_assert(sizeof(GpuContactMetrics)==80);
static_assert(offsetof(GpuContactMetrics,known)==48);
extern "C" void gpu_b3_world_contact_metrics(b3WorldId,GpuContactMetrics*);

static_assert(offsetof(GpuContactMetrics,non_sensor_roots)==76);
