#pragma once

#include "box3d/id.h"

#include <stdbool.h>

void both_map_world(b3WorldId gpu, b3WorldId cpu);
void both_map_body(b3BodyId gpu, b3BodyId cpu);
void both_map_shape(b3ShapeId gpu, b3ShapeId cpu);
void both_map_joint(b3JointId gpu, b3JointId cpu);

void both_unmap_world(b3WorldId gpu);
void both_unmap_body(b3BodyId gpu);
void both_unmap_shape(b3ShapeId gpu);
void both_unmap_joint(b3JointId gpu);

b3WorldId both_cpu_world(b3WorldId gpu);
b3BodyId both_cpu_body(b3BodyId gpu);
b3ShapeId both_cpu_shape(b3ShapeId gpu);
b3JointId both_cpu_joint(b3JointId gpu);

bool both_has_cpu_world(b3WorldId gpu);
bool both_has_cpu_body(b3BodyId gpu);
bool both_has_cpu_shape(b3ShapeId gpu);
bool both_has_cpu_joint(b3JointId gpu);
void both_clear_maps(void);
