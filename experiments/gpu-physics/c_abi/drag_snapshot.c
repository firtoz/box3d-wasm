// Diagnostic-only exporter linked against the prefixed CPU oracle.
#include "body.h"
#include "box3d/box3d.h"
#include "contact.h"
#include "joint.h"
#include "physics_world.h"
#include "solver_set.h"
#include <assert.h>
#include <stdio.h>
static void v3(FILE *f, b3Vec3 v) {
  fprintf(f, "[%.9g,%.9g,%.9g]", v.x, v.y, v.z);
}
static void q4(FILE *f, b3Quat q) {
  fprintf(f, "[%.9g,%.9g,%.9g,%.9g]", q.v.x, q.v.y, q.v.z, q.s);
}
void drag_export_snapshot(b3WorldId id, b3JointId jid, int gpu_joint_slot,
                          const char *path) {
  b3World *world = b3GetWorldFromId(id);
  FILE *f = fopen(path, "w");
  assert(f);
  fprintf(f, "{\"version\":1,\"bodies\":[");
  int n = 0;
  for (int i = 0; i < world->bodies.count; ++i) {
    b3Body *b = world->bodies.data + i;
    if (b->setIndex == B3_NULL_INDEX)
      continue;
    b3BodySim *s = b3GetBodySim(world, b);
    b3BodyState *state = b3GetBodyState(world, b);
    assert(b3LengthSquared(s->localCenter) == 0 &&
           b3LengthSquared(s->force) == 0 && b3LengthSquared(s->torque) == 0);
    assert(s->invInertiaLocal.cx.y == 0 && s->invInertiaLocal.cx.z == 0 &&
           s->invInertiaLocal.cy.z == 0);
    assert(s->linearDamping == 0 && s->angularDamping == 0);
    if (n++)
      fprintf(f, ",");
    fprintf(f, "{\"id\":%d,\"p\":[%.9g,%.9g,%.9g],\"q\":", i, s->center.x,
            s->center.y, s->center.z);
    q4(f, s->transform.q);
    fprintf(f, ",\"v\":");
    v3(f, state ? state->linearVelocity : b3Vec3_zero);
    fprintf(f, ",\"w\":");
    v3(f, state ? state->angularVelocity : b3Vec3_zero);
    fprintf(f,
            ",\"inv_mass\":%.9g,\"inv_inertia\":[%.9g,%.9g,%.9g],\"awake\":%d}",
            s->invMass, s->invInertiaLocal.cx.x, s->invInertiaLocal.cy.y,
            s->invInertiaLocal.cz.z, b3Body_IsAwake(b3MakeBodyId(world, i)));
  }
  fprintf(f, "],\"contacts\":[");
  n = 0;
  for (int i = 0; i < world->contacts.count; ++i) {
    b3Contact *c = world->contacts.data + i;
    if (c->setIndex == B3_NULL_INDEX)
      continue;
    assert(!(c->flags & b3_simMeshContact));
    assert(c->manifoldCount == 1); // bounded replay refuses omitted caches
    b3Manifold *m = c->manifolds;
    assert(m->pointCount > 0 && m->pointCount <= 4);
    if (n++)
      fprintf(f, ",");
    fprintf(f, "{\"a\":%d,\"b\":%d,\"color\":%d,\"local\":%d,\"normal\":",
            c->edges[0].bodyId, c->edges[1].bodyId, c->colorIndex,
            c->localIndex);
    v3(f, m->normal);
    b3Vec3 t = b3Perp(m->normal);
    b3Vec3 t2 = b3Cross(t, m->normal);
    fprintf(
        f,
        ",\"friction_impulse\":[%.9g,%.9g],\"twist\":%.9g,\"rolling_impulse\":",
        b3Dot(m->frictionImpulse, t), b3Dot(m->frictionImpulse, t2),
        m->twistImpulse);
    v3(f, m->rollingImpulse);
    fprintf(f,
            ",\"material\":[%.9g,%.9g,%.9g],\"tangent_velocity\":", c->friction,
            c->restitution, c->rollingResistance);
    v3(f, c->tangentVelocity);
    fprintf(f, ",\"qa\":");
    q4(f, c->cachedRotationA);
    fprintf(f, ",\"qb\":");
    q4(f, c->cachedRotationB);
    fprintf(f, ",\"relative\":");
    v3(f, c->cachedRelativePose.p);
    b3SATCache cache = c->convexContact.cache.satCache;
    fprintf(f, ",\"sat\":[%u,%u,%u,%.9g],\"cache_valid\":%d,\"points\":[",
            cache.type, cache.indexA, cache.indexB, cache.separation,
            (c->flags & b3_relativeTransformValid) != 0);
    for (int j = 0; j < m->pointCount; ++j) {
      b3ManifoldPoint *p = m->points + j;
      if (j)
        fprintf(f, ",");
      fprintf(f, "{\"ra\":");
      v3(f, p->anchorA);
      fprintf(f, ",\"rb\":");
      v3(f, p->anchorB);
      fprintf(f,
              ",\"separation\":%.9g,\"base\":%.9g,\"impulse\":%.9g,\"feature\":"
              "%u,\"persisted\":%d}",
              p->separation, p->baseSeparation, p->normalImpulse, p->featureId,
              p->persisted);
    }
    fprintf(f, "]}");
  }
  if (jid.index1 == 0) {
    fprintf(f, "],\"joint\":null}\n");
    assert(fclose(f) == 0);
    return;
  }
  b3JointSim *j = b3GetJointSimCheckType(jid, b3_motorJoint);
  b3MotorJoint *m = &j->motorJoint;
  fprintf(f, "],\"joint\":{\"slot\":%d,\"a\":%d,\"b\":%d,\"anchor_a\":",
          gpu_joint_slot, j->bodyIdA, j->bodyIdB);
  v3(f, j->localFrameA.p);
  fprintf(f, ",\"anchor_b\":");
  v3(f, j->localFrameB.p);
  fprintf(f, ",\"frame_a\":");
  q4(f, j->localFrameA.q);
  fprintf(f, ",\"frame_b\":");
  q4(f, j->localFrameB.q);
  fprintf(f, ",\"linear_velocity\":");
  v3(f, m->linearVelocity);
  fprintf(f, ",\"angular_velocity\":");
  v3(f, m->angularVelocity);
  fprintf(f, ",\"tuning\":[%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g],\"lv\":",
          m->linearHertz, m->linearDampingRatio, m->maxSpringForce,
          m->maxVelocityForce, m->angularHertz, m->angularDampingRatio,
          m->maxSpringTorque, m->maxVelocityTorque);
  v3(f, m->linearVelocityImpulse);
  fprintf(f, ",\"ls\":");
  v3(f, m->linearSpringImpulse);
  fprintf(f, ",\"av\":");
  v3(f, m->angularVelocityImpulse);
  fprintf(f, ",\"as\":");
  v3(f, m->angularSpringImpulse);
  fprintf(f, "}}\n");
  assert(fclose(f) == 0);
}
