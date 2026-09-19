#include <climits>
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#ifdef GPU_REFERENCE_BOTH
// These headless fixtures link the dual-world bridge but do not draw selections.
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif
#ifdef GPU_REFERENCE_BOTH
extern "C" {
#include "both_ids.h"
}
#endif
static int fault, creates, attaches;
extern "C" {
const char *gpu_b3_world_gpu_fail(b3WorldId);
b3ShapeId __real_gpu_b3_create_compound_parent(b3BodyId, float, float, float,
                                               float, bool);
b3ShapeId __wrap_gpu_b3_create_compound_parent(b3BodyId b, float d, float f,
                                               float r, float roll,
                                               bool sensor) {
  return fault == 1
             ? b3ShapeId{}
             : __real_gpu_b3_create_compound_parent(b, d, f, r, roll, sensor);
}
b3ShapeId __real_gpu_b3_create_sphere(b3BodyId, float, float, float, float,
                                      float, float, float, float, bool);
b3ShapeId __wrap_gpu_b3_create_sphere(b3BodyId b, float x, float y, float z,
                                      float radius, float d, float f, float r,
                                      float roll, bool mass) {
  ++creates;
  if ((fault == 2 && creates == 1) || (fault == 3 && creates == 2))
    return {};
  return __real_gpu_b3_create_sphere(b, x, y, z, radius, d, f, r, roll, mass);
}
bool __real_gpu_b3_shape_attach_compound_child_materials(b3ShapeId, b3ShapeId,
                                                         int, const int *);
bool __wrap_gpu_b3_shape_attach_compound_child_materials(b3ShapeId p,
                                                         b3ShapeId c, int i,
                                                         const int *m) {
  ++attaches;
  if ((fault == 4 && attaches == 1) || (fault == 5 && attaches == 2))
    return false;
  return __real_gpu_b3_shape_attach_compound_child_materials(p, c, i, m);
}
}
int main() {
  int failures = 0;
  for (fault = 0; fault <= 5; ++fault) {
    creates = attaches = 0;
    auto wd = b3DefaultWorldDef();
    auto world = b3CreateWorld(&wd);
#ifdef GPU_REFERENCE_BOTH
    if (!both_has_cpu_world(world))
      return 93;
#endif
    auto bd = b3DefaultBodyDef();
    auto body = b3CreateBody(world, &bd);
    b3CompoundSphereDef spheres[2] = {};
    for (int i = 0; i < 2; ++i) {
      spheres[i].sphere = {{float(i) * 2, 0, 0}, .5f};
      spheres[i].material = b3DefaultSurfaceMaterial();
    }
    b3CompoundDef cd = {};
    cd.spheres = spheres;
    cd.sphereCount = 2;
    auto compound = b3CreateCompound(&cd);
    auto sd = b3DefaultShapeDef();
    auto shape = b3CreateBakedCompoundShape(body, &sd, compound);
    const char *reason = gpu_b3_world_gpu_fail(world);
    bool invalid = reason && *reason;
    bool ok =
        fault ? shape.index1 == 0 && invalid : shape.index1 > 0 && !invalid;
    printf("fault %d shape %d invalid %d ok %d\n", fault, shape.index1 != 0,
           invalid, ok);
    failures += !ok;
    b3DestroyWorld(world);
    b3DestroyCompound(compound);
    if (b3World_IsValid(world))
      return 94;
  }
  // Invalid counts must be rejected before native code reads child pointers
  // or computes allocation sizes. Include a sum that would overflow int32.
  for (int i = 0; i < 3; ++i) {
    auto wd = b3DefaultWorldDef();
    auto world = b3CreateWorld(&wd);
    const char* initialReason = gpu_b3_world_gpu_fail(world);
    if (initialReason && *initialReason) return 95; // Previous rejection must not poison a new world.
    b3CompoundDef cd = {};
    cd.sphereCount = i == 0 ? -1 : (i == 1 ? INT_MAX : 65536);
    cd.hullCount = i == 1 ? INT_MAX : 0;
    auto compound = b3CreateCompound(&cd);
    auto bd = b3DefaultBodyDef();
    auto body = b3CreateBody(world, &bd);
    auto sd = b3DefaultShapeDef();
    auto shape = b3CreateBakedCompoundShape(body, &sd, compound);
    const char* reason = gpu_b3_world_gpu_fail(world);
    bool ok = shape.index1 == 0 && reason && *reason;
    printf("count_guard %d ok %d\n", i, ok);
    failures += !ok;
    b3DestroyCompound(compound);
    b3DestroyWorld(world);
  }
  return failures ? 32 : 0;
}
