// CPU contract for
// point_persistence_survives_publication_recycling_and_zero_impulses.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cmath>
#include <cstdio>
#include <cstddef>
// Keep these checks paired with api::contact_data::tests::native_contact_layout_matches_box3d.
static_assert(sizeof(b3ManifoldPoint) == 56 && alignof(b3ManifoldPoint) == 4);
static_assert(offsetof(b3ManifoldPoint, anchorB) == 12);
static_assert(offsetof(b3ManifoldPoint, separation) == 24);
static_assert(offsetof(b3ManifoldPoint, baseSeparation) == 28);
static_assert(offsetof(b3ManifoldPoint, normalImpulse) == 32);
static_assert(offsetof(b3ManifoldPoint, totalNormalImpulse) == 36);
static_assert(offsetof(b3ManifoldPoint, normalVelocity) == 40);
static_assert(offsetof(b3ManifoldPoint, featureId) == 44);
static_assert(offsetof(b3ManifoldPoint, triangleIndex) == 48);
static_assert(offsetof(b3ManifoldPoint, persisted) == 52);
static_assert(sizeof(b3Manifold) == 268 && alignof(b3Manifold) == 4);
static_assert(offsetof(b3Manifold, normal) == 224);
static_assert(offsetof(b3Manifold, twistImpulse) == 236);
static_assert(offsetof(b3Manifold, frictionImpulse) == 240);
static_assert(offsetof(b3Manifold, rollingImpulse) == 252);
static_assert(offsetof(b3Manifold, pointCount) == 264);
static_assert(offsetof(b3ContactData, shapeIdA) == 12);
static_assert(offsetof(b3ContactData, shapeIdB) == 20);
static_assert(sizeof(void*) != 8 || (sizeof(b3ContactData) == 48 && alignof(b3ContactData) == 8));
static_assert(sizeof(void*) != 8 || offsetof(b3ContactData, manifolds) == 32);
static_assert(sizeof(void*) != 8 || offsetof(b3ContactData, manifoldCount) == 40);
int main() {
  b3WorldDef wd = b3DefaultWorldDef();
  wd.gravity = b3Vec3_zero;
  wd.enableSleep = false;
  auto world = b3CreateWorld(&wd);
  auto gd = b3DefaultBodyDef();
  auto ground = b3CreateBody(world, &gd);
  auto sd = b3DefaultShapeDef();
  auto floor = b3MakeBoxHull(4.0f, 0.5f, 4.0f);
  b3CreateHullShape(ground, &sd, &floor.base);
  auto bd = b3DefaultBodyDef();
  bd.type = b3_dynamicBody;
  bd.position = {0.0f, 1.0f, 0.0f};
  auto body = b3CreateBody(world, &bd);
  auto hull = b3MakeBoxHull(0.5f, 0.5f, 0.5f);
  b3CreateHullShape(body, &sd, &hull.base);
  for (int step = 0; step < 3; step++) {
    if (step == 2)
      b3Body_EnableContactRecycling(body, false);
    b3World_Step(world, 1.0f / 60.0f, 4);
    b3ContactData contact[2];
    int count = b3Body_GetContactData(body, contact, 2);
    if (count != 1 || contact[0].manifoldCount != 1)
      return 2;
    const auto &m = contact[0].manifolds[0];
    if (m.pointCount != 4)
      return 3;
    for (int point = 0; point < 4; point++) {
      const auto &p = m.points[point];
      std::printf("step=%d point=%d persisted=%d impulse=%.9g\n", step, point,
                  int(p.persisted), p.normalImpulse);
      if (std::fabs(p.separation) > 1e-6f || std::fabs(p.baseSeparation) > 1e-6f)
        return 6;
      if (p.totalNormalImpulse != 0.0f || p.normalVelocity != 0.0f || p.triangleIndex != -1)
        return 7;
      if (p.persisted != (step > 0))
        return 4;
      if (std::fabs(p.normalImpulse) > 1e-7f)
        return 5;
    }
  }
  b3DestroyWorld(world);
  return 0;
}
