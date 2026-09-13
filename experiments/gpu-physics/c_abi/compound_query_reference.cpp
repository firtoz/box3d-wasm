#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
struct Hits {
  int calls = 0, planes = 0;
  bool identity = true;
  b3ShapeId expected;
  float fraction = 1;
  int child = -1;
  uint64_t material = 0;
};
static bool Overlap(b3ShapeId id, void *data) {
  auto *h = (Hits *)data;
  ++h->calls;
  h->identity &= B3_ID_EQUALS(id, h->expected);
  return true;
}
static bool Filter(b3ShapeId id, void *data) { return Overlap(id, data); }
static float Ray(b3ShapeId id, b3Pos, b3Vec3, float fraction, uint64_t material,
                 int, int child, void *data) {
  auto *h = (Hits *)data;
  ++h->calls;
  h->identity &= B3_ID_EQUALS(id, h->expected);
  h->fraction = fraction;
  h->material = material;
  h->child = child;
  return 1;
}
static bool Mover(b3ShapeId id, const b3PlaneResult *, int count, void *data) {
  auto *h = (Hits *)data;
  ++h->calls;
  h->planes += count;
  h->identity &= B3_ID_EQUALS(id, h->expected);
  return true;
}
int main() {
  auto wd = b3DefaultWorldDef();
  auto w = b3CreateWorld(&wd);
  auto bd = b3DefaultBodyDef();
  auto b = b3CreateBody(w, &bd);
  b3CompoundSphereDef spheres[2] = {};
  for (int i = 0; i < 2; i++) {
    spheres[i].sphere = {{i ? .5f : -.5f, 0, 0}, 1};
    spheres[i].material = b3DefaultSurfaceMaterial();
    spheres[i].material.userMaterialId = 100 + i;
  }
  b3CompoundDef cd = {};
  cd.spheres = spheres;
  cd.sphereCount = 2;
  auto compound = b3CreateCompound(&cd);
  auto sd = b3DefaultShapeDef();
  auto shape = b3CreateBakedCompoundShape(b, &sd, compound);
  Hits overlap = {}, aabb = {}, ray = {}, mover = {}, cast = {}, filter = {};
  overlap.expected = aabb.expected = ray.expected = mover.expected =
      cast.expected = filter.expected = shape;
  b3Vec3 point = {0, 0, 0};
  b3ShapeProxy proxy = {&point, 1, .25f};
  b3World_OverlapShape(w, b3Pos_zero, &proxy, b3DefaultQueryFilter(), Overlap,
                       &overlap);
  b3World_OverlapAABB(w, {{-2, -2, -2}, {2, 2, 2}}, b3DefaultQueryFilter(),
                      Overlap, &aabb);
  b3World_CastRay(w, {-3, 0, 0}, {6, 0, 0}, b3DefaultQueryFilter(), Ray, &ray);
  b3Capsule capsule = {{0, 0, 0}, {0, 0, 0}, .5f};
  b3World_CollideMover(w, {0, 1, 0}, &capsule, b3DefaultQueryFilter(), Mover,
                       &mover);
  b3World_CastShape(w, {-3, 0, 0}, &proxy, {6, 0, 0}, b3DefaultQueryFilter(),
                    Ray, &cast);
  b3World_CastMover(w, {-3, 0, 0}, &capsule, {6, 0, 0}, b3DefaultQueryFilter(),
                    Filter, &filter);
  printf("%d %d %d %d %d %d %d %d\n", overlap.calls, aabb.calls, ray.calls,
         mover.calls, mover.planes, cast.calls, filter.calls,
         overlap.identity && aabb.identity && ray.identity && mover.identity &&
             cast.identity && filter.identity);
  printf("ray-left %d %.9g %d %llu\n", ray.calls, ray.fraction, ray.child,
         (unsigned long long)ray.material);
  ray = Hits{};
  ray.expected = shape;
  b3World_CastRay(w, {3, 0, 0}, {-6, 0, 0}, b3DefaultQueryFilter(), Ray, &ray);
  printf("ray-right %d %.9g %d %llu %d\n", ray.calls, ray.fraction, ray.child,
         (unsigned long long)ray.material, ray.identity);
  b3DestroyWorld(w);
  b3DestroyCompound(compound);
}
