#include "shape_replacement_schedule.h"
extern "C" {
#include "both_ids.h"
b3MassData cpu_b3Body_GetMassData(b3BodyId);
b3Pos cpu_b3Body_GetPosition(b3BodyId);
b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId);
b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId);
b3Quat cpu_b3Body_GetRotation(b3BodyId);
b3ShapeType cpu_b3Shape_GetType(b3ShapeId);
b3WorldCastOutput cpu_b3Shape_RayCast(b3ShapeId, b3Pos, b3Vec3);
b3AABB cpu_b3Shape_GetAABB(b3ShapeId);
b3RayResult cpu_b3World_CastRayClosest(b3WorldId, b3Pos, b3Vec3, b3QueryFilter);
void *cpu_b3Shape_GetUserData(b3ShapeId);
float cpu_b3Shape_GetDensity(b3ShapeId);
b3Filter cpu_b3Shape_GetFilter(b3ShapeId);
b3SurfaceMaterial cpu_b3Shape_GetSurfaceMaterial(b3ShapeId);
const b3HullData *cpu_b3Shape_GetHull(b3ShapeId);
b3ContactEvents cpu_b3World_GetContactEvents(b3WorldId);
int cpu_b3Shape_GetContactData(b3ShapeId, b3ContactData *, int);
bool cpu_b3Contact_IsValid(b3ContactId);
bool cpu_b3Body_IsAwake(b3BodyId);
const char *gpu_samples_shape_get_name(b3ShapeId);
void cpu_b3Shape_SetHull(b3ShapeId, const b3HullData *);
void gpu_shape_set_hull(b3ShapeId, const b3HullData *);
int gpu_shape_geometry_mirror_count(void);
void gpu_samples_world_draw(b3WorldId, b3DebugDraw *, uint64_t);
void cpu_b3World_Draw(b3WorldId, b3DebugDraw *, uint64_t);
void gpu_b3_world_wait(b3WorldId);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
struct Visual {
  b3ShapeType type;
  float radius;
  uint64_t hash;
};
static int visualLive = 0;
static Visual drawn;
static void *createVisual(const b3DebugShape *shape, void *) {
  auto *v = new Visual{shape->type, 0, 0};
  if (shape->type == b3_sphereShape)
    v->radius = shape->sphere->radius;
  if (shape->type == b3_capsuleShape)
    v->radius = shape->capsule->radius;
  if (shape->type == b3_hullShape)
    v->hash = shape->hull->hash;
  ++visualLive;
  return v;
}
static void destroyVisual(void *value, void *) {
  --visualLive;
  delete static_cast<Visual *>(value);
}
static void drawVisual(void *value, b3WorldTransform, b3HexColor, void *) {
  drawn = *static_cast<Visual *>(value);
}
static void compareVisuals(b3WorldId w) {
  b3DebugDraw draw = {};
  draw.drawShapes = true;
  draw.DrawShapeFcn = drawVisual;
  draw.drawingBounds = {{-100, -100, -100}, {100, 100, 100}};
  drawn = {};
  gpu_samples_world_draw(w, &draw, UINT64_MAX);
  auto gpu = drawn;
  drawn = {};
  cpu_b3World_Draw(both_cpu_world(w), &draw, UINT64_MAX);
  auto cpu = drawn;
  assert(gpu.type == cpu.type && gpu.radius == cpu.radius &&
         gpu.hash == cpu.hash);
  assert(visualLive == 2);
}
static void near(float a, float b, float tolerance = 1e-5f) {
  if (!(fabsf(a - b) <= tolerance * fmaxf(1.0f, fmaxf(fabsf(a), fabsf(b))))) {
    fprintf(stderr, "mismatch %.9g %.9g tolerance %g\n", a, b, tolerance);
    abort();
  }
}
static void mass_equal(b3MassData a, b3MassData b) {
  near(a.mass, b.mass);
  near(a.center.x, b.center.x);
  near(a.center.y, b.center.y);
  near(a.center.z, b.center.z);
  const float *x = &a.inertia.cx.x;
  const float *y = &b.inertia.cx.x;
  for (int i = 0; i < 9; ++i)
    near(x[i], y[i]);
}
static void inspect(b3ShapeId s, b3BodyId b) {
  auto cs = both_cpu_shape(s);
  auto cb = both_cpu_body(b);
  assert(b3Shape_IsValid(s));
  assert(strcmp(gpu_samples_shape_get_name(s), "stable shape") == 0);
  assert(b3Shape_GetType(s) == cpu_b3Shape_GetType(cs));
  assert(b3Shape_GetUserData(s) == cpu_b3Shape_GetUserData(cs));
  assert(b3Shape_GetUserData(s) == reinterpret_cast<void *>(123));
  near(b3Shape_GetDensity(s), cpu_b3Shape_GetDensity(cs));
  assert(b3Shape_AreContactEventsEnabled(s));
  auto material = b3Shape_GetSurfaceMaterial(s),
       cm = cpu_b3Shape_GetSurfaceMaterial(cs);
  near(material.friction, cm.friction);
  near(material.restitution, cm.restitution);
  near(material.rollingResistance, cm.rollingResistance);
  assert(material.userMaterialId == cm.userMaterialId &&
         material.customColor == cm.customColor);
  b3ShapeId owned[2];
  assert(b3Body_GetShapes(b, owned, 2) == 1 && B3_ID_EQUALS(owned[0], s));
  auto f = b3Shape_GetFilter(s), cf = cpu_b3Shape_GetFilter(cs);
  assert(f.categoryBits == cf.categoryBits && f.maskBits == cf.maskBits &&
         f.groupIndex == cf.groupIndex);
  mass_equal(b3Body_GetMassData(b), cpu_b3Body_GetMassData(cb));
  auto a = b3Shape_GetAABB(s), ca = cpu_b3Shape_GetAABB(cs);
  near(a.lowerBound.x, ca.lowerBound.x);
  near(a.lowerBound.y, ca.lowerBound.y);
  near(a.lowerBound.z, ca.lowerBound.z);
  near(a.upperBound.x, ca.upperBound.x);
  near(a.upperBound.y, ca.upperBound.y);
  near(a.upperBound.z, ca.upperBound.z);
  auto p = b3Body_GetPosition(b);
  b3Pos origin = {p.x, p.y, p.z + 10};
  b3Vec3 delta = {0, 0, -20};
  auto hit = b3Shape_RayCast(s, origin, delta),
       ch = cpu_b3Shape_RayCast(cs, origin, delta);
  assert(hit.hit == ch.hit);
  if (hit.hit)
    near(hit.fraction, ch.fraction);
  auto world = b3Shape_GetWorld(s);
  auto ray =
      b3World_CastRayClosest(world, origin, delta, b3DefaultQueryFilter());
  auto cpuRay = cpu_b3World_CastRayClosest(both_cpu_world(world), origin, delta,
                                           b3DefaultQueryFilter());
  assert(ray.hit == cpuRay.hit);
  if (ray.hit) {
    near(ray.fraction, cpuRay.fraction);
    assert(B3_ID_EQUALS(ray.shapeId, s));
  }
}
static void step(b3WorldId w) {
  b3World_Step(w, 1.0f / 60, 4);
  gpu_b3_world_wait(w);
}
int main() {
  auto wd = b3DefaultWorldDef();
  wd.gravity = {0, 0, 0};
  wd.createDebugShape = createVisual;
  wd.destroyDebugShape = destroyVisual;
  auto w = b3CreateWorld(&wd);
  auto bd = b3DefaultBodyDef();
  bd.type = b3_dynamicBody;
  bd.position = {0, 3, 0};
  auto b = b3CreateBody(w, &bd);
  auto sd = b3DefaultShapeDef();
  sd.userData = reinterpret_cast<void *>(123);
  sd.enableContactEvents = true;
  sd.density = 2.5f;
  sd.baseMaterial.friction = .35f;
  sd.baseMaterial.restitution = .2f;
  sd.baseMaterial.rollingResistance = .05f;
  sd.baseMaterial.userMaterialId = 17;
  sd.baseMaterial.customColor = 0x123456;
  sd.filter.categoryBits = 4;
  auto box = b3MakeBoxHull(.5f, .5f, .5f);
  auto s = b3CreateHullShape(b, &sd, &box.base);
  b3Shape_SetName(s, "stable shape");
  auto original = s;
  auto originalMass = b3Body_GetMassData(b);
  b3Body_SetAwake(b, false);
  b3Sphere sphere = {{.3f, 0, 0}, .7f};
  b3Shape_SetSphere(s, &sphere);
  assert(!b3Body_IsAwake(b) && !cpu_b3Body_IsAwake(both_cpu_body(b)));
  mass_equal(originalMass, b3Body_GetMassData(b));
  inspect(s, b);
  b3Body_ApplyMassFromShapes(b);
  inspect(s, b);
  for (int phase = 0; phase < 32; ++phase) {
    auto before = b3Body_GetMassData(b);
    replace_sample_shape(s, b, phase);
    assert(memcmp(&original, &s, sizeof(s)) == 0);
    if (!(phase % 2))
      mass_equal(before, b3Body_GetMassData(b));
    inspect(s, b);
    compareVisuals(w);
    if (b3Shape_GetType(s) == b3_hullShape) {
      auto h = b3Shape_GetHull(s);
      auto ch = cpu_b3Shape_GetHull(both_cpu_shape(s));
      assert(h && ch && h != ch);
      assert(h->hash == ch->hash);
      gpu_shape_set_hull(s, h);
      cpu_b3Shape_SetHull(both_cpu_shape(s), ch);
      assert(b3Shape_GetHull(s) == h);
      inspect(s, b);
    }
    step(w);
    inspect(s, b);
  }
  b3Body_SetAwake(b, true);
  b3Body_SetLinearVelocity(b, {0.15f, 0.1f, -0.1f});
  b3Body_SetAngularVelocity(b, {.1f, .2f, .15f});
  for (int frame = 0; frame < 120; ++frame) {
    if (frame % 15 == 0)
      replace_sample_shape(s, b, frame / 15);
    step(w);
    auto gp = b3Body_GetPosition(b),
         cp = cpu_b3Body_GetPosition(both_cpu_body(b));
    near(gp.x, cp.x);
    near(gp.y, cp.y);
    near(gp.z, cp.z);
    auto gv = b3Body_GetLinearVelocity(b),
         cv = cpu_b3Body_GetLinearVelocity(both_cpu_body(b));
    near(gv.x, cv.x);
    near(gv.y, cv.y);
    near(gv.z, cv.z);
    auto ga = b3Body_GetAngularVelocity(b),
         ca = cpu_b3Body_GetAngularVelocity(both_cpu_body(b));
    near(ga.x, ca.x);
    near(ga.y, ca.y);
    near(ga.z, ca.z);
    auto gq = b3Body_GetRotation(b),
         cq = cpu_b3Body_GetRotation(both_cpu_body(b));
    near(gq.v.x, cq.v.x);
    near(gq.v.y, cq.v.y);
    near(gq.v.z, cq.v.z);
    near(gq.s, cq.s);
  }
  // Input can belong to another shape; destroying that owner cannot free ours.
  auto other = b3CreateBody(w, &bd);
  auto otherShape = b3CreateHullShape(other, &sd, &box.base);
  b3Shape_SetHull(s, b3Shape_GetHull(otherShape));
  b3DestroyBody(other);
  assert(b3Shape_GetHull(s) && b3Shape_GetHull(s)->hash == box.base.hash);
  b3DestroyBody(b);
  assert(gpu_shape_geometry_mirror_count() == 0);
  b3DestroyWorld(w);
  assert(gpu_shape_geometry_mirror_count() == 0);
  assert(visualLive == 0);

  // Existing touching contacts, sleeping pair, same-hull no-op, and retirement.
  w = b3CreateWorld(&wd);
  bd = b3DefaultBodyDef();
  auto ground = b3CreateBody(w, &bd);
  b3CreateHullShape(ground, &sd, &box.base);
  bd.type = b3_dynamicBody;
  bd.position = {0, .9f, 0};
  b = b3CreateBody(w, &bd);
  s = b3CreateHullShape(b, &sd, &box.base);
  auto offset = b3MakeOffsetBoxHull(.5f, .5f, .5f, {3, 0, 0});
  b3CreateHullShape(ground, &sd, &offset.base);
  bd.position = {3, .9f, 0};
  auto spectator = b3CreateBody(w, &bd);
  auto spectatorShape = b3CreateHullShape(spectator, &sd, &box.base);
  bd.position = {5, .9f, 0};
  auto linked = b3CreateBody(w, &bd);
  b3CreateHullShape(linked, &sd, &box.base);
  auto weld = b3DefaultWeldJointDef();
  weld.base.bodyIdA = b;
  weld.base.bodyIdB = linked;
  weld.base.localFrameA.p = {5, 0, 0};
  b3CreateWeldJoint(w, &weld);
  step(w);
  b3ContactData untouchedGpu[4], untouchedCpu[4];
  assert(b3Shape_GetContactData(spectatorShape, untouchedGpu, 4) == 1);
  assert(cpu_b3Shape_GetContactData(both_cpu_shape(spectatorShape),
                                    untouchedCpu, 4) == 1);
  b3ContactData g[4], c[4];
  assert(b3Shape_GetContactData(s, g, 4) == 1);
  assert(cpu_b3Shape_GetContactData(both_cpu_shape(s), c, 4) == 1);
  b3Body_SetAwake(b, false);
  b3Body_SetAwake(linked, false);
  auto h = b3Shape_GetHull(s);
  b3Shape_SetHull(s, h);
  assert(b3Contact_IsValid(g[0].contactId));
  assert(cpu_b3Contact_IsValid(c[0].contactId));
  assert(!b3Body_IsAwake(b) && !cpu_b3Body_IsAwake(both_cpu_body(b)));
  sphere = {{0, 0, 0}, .2f};
  b3Shape_SetSphere(s, &sphere);
  assert(b3Body_IsAwake(b) && cpu_b3Body_IsAwake(both_cpu_body(b)));
  assert(b3Body_IsAwake(linked) && cpu_b3Body_IsAwake(both_cpu_body(linked)));
  assert(!b3Contact_IsValid(g[0].contactId));
  assert(!cpu_b3Contact_IsValid(c[0].contactId));
  assert(b3Contact_IsValid(untouchedGpu[0].contactId));
  assert(cpu_b3Contact_IsValid(untouchedCpu[0].contactId));
  assert(b3Shape_GetContactData(s, g, 4) == 0);
  assert(cpu_b3Shape_GetContactData(both_cpu_shape(s), c, 4) == 0);
  step(w);
  auto ge = b3World_GetContactEvents(w),
       ce = cpu_b3World_GetContactEvents(both_cpu_world(w));
  assert(ge.endCount == ce.endCount && ge.endCount == 1);
  b3Capsule capsule = {{0, -.25f, 0}, {0, .25f, 0}, .5f};
  b3Shape_SetCapsule(s, &capsule);
  step(w);
  assert(b3Shape_GetContactData(s, g, 4) == 1);
  assert(cpu_b3Shape_GetContactData(both_cpu_shape(s), c, 4) == 1);
  auto wide = b3MakeBoxHull(.6f, .8f, .6f);
  b3Shape_SetHull(s, &wide.base);
  assert(!b3Contact_IsValid(g[0].contactId));
  assert(!cpu_b3Contact_IsValid(c[0].contactId));
  assert(b3Contact_IsValid(untouchedGpu[0].contactId));
  assert(cpu_b3Contact_IsValid(untouchedCpu[0].contactId));
  step(w);
  ge = b3World_GetContactEvents(w);
  ce = cpu_b3World_GetContactEvents(both_cpu_world(w));
  assert(ge.beginCount == ce.beginCount && ge.endCount == ce.endCount);
  b3Body_ApplyMassFromShapes(b);
  mass_equal(b3Body_GetMassData(b), cpu_b3Body_GetMassData(both_cpu_body(b)));
  b3DestroyShape(s, false);
  assert(!b3Shape_IsValid(s));
  b3DestroyWorld(w);
  assert(gpu_shape_geometry_mirror_count() == 0);
  puts("shape replacement: independent CPU/GPU identity, metadata, mass, "
       "geometry, queries, aliasing, sleep and contacts passed");
}
