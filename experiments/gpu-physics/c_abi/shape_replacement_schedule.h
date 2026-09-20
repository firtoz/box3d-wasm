#pragma once
#include "box3d/box3d.h"

// Shared input schedule; each native engine owns and evolves its own state.
static inline void replace_sample_shape(b3ShapeId shape, b3BodyId body,
                                        int phase) {
  switch (phase % 4) {
  case 0: {
    b3Sphere sphere = {{0.25f, 0.0f, 0.0f}, 0.65f};
    b3Shape_SetSphere(shape, &sphere);
    break;
  }
  case 1: {
    b3Capsule capsule = {{0.0f, -0.5f, 0.0f}, {0.0f, 0.5f, 0.0f}, 0.35f};
    b3Shape_SetCapsule(shape, &capsule);
    break;
  }
  case 2: {
    b3BoxHull box = b3MakeBoxHull(0.6f, 0.4f, 0.5f);
    b3Shape_SetHull(shape, &box.base);
    break;
  }
  case 3: {
    b3HullData *hull = b3CreateCylinder(1.2f, 0.45f, -0.6f, 8);
    b3Shape_SetHull(shape, hull);
    b3DestroyHull(hull);
    break;
  }
  }
  if (phase % 2) {
    b3Body_ApplyMassFromShapes(body);
  }
}
