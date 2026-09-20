#include "box3d/box3d.h"
extern "C" {
#include "both_ids.h"
float cpu_b3Joint_GetLinearSeparation(b3JointId);
float cpu_b3Joint_GetAngularSeparation(b3JointId);
void cpu_b3Body_SetTransform(b3BodyId, b3Pos, b3Quat);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
static int kind, config, pose, checks;
static b3WorldId world;
static void near(float g, float c) {
  if (!(std::isfinite(g) && std::isfinite(c) &&
        fabsf(g - c) <= 1e-5f * fmaxf(1, fmaxf(fabsf(g), fabsf(c))))) {
    fprintf(stderr, "kind=%d config=%d pose=%d GPU=%.9g CPU=%.9g\n", kind,
            config, pose, g, c);
    abort();
  }
  ++checks;
}
static void inspect(b3JointId j) {
  near(b3Joint_GetLinearSeparation(j),
       cpu_b3Joint_GetLinearSeparation(both_cpu_joint(j)));
  if (kind != 8)
    near(b3Joint_GetAngularSeparation(j),
         cpu_b3Joint_GetAngularSeparation(both_cpu_joint(j)));
  else
    assert(b3Joint_GetAngularSeparation(j) == 0);
}
static b3JointId create(b3JointDef base) {
  switch (kind) {
  case 0: {
    auto d = b3DefaultDistanceJointDef();
    d.base = base;
    d.length = 2;
    d.minLength = 1;
    d.maxLength = 3;
    d.enableSpring = config & 1;
    d.enableLimit = config & 2;
    return b3CreateDistanceJoint(world, &d);
  }
  case 1: {
    auto d = b3DefaultMotorJointDef();
    d.base = base;
    return b3CreateMotorJoint(world, &d);
  }
  case 2: {
    auto d = b3DefaultFilterJointDef();
    d.base = base;
    return b3CreateFilterJoint(world, &d);
  }
  case 3: {
    auto d = b3DefaultParallelJointDef();
    d.base = base;
    return b3CreateParallelJoint(world, &d);
  }
  case 4: {
    auto d = b3DefaultPrismaticJointDef();
    d.base = base;
    d.enableLimit = config & 1;
    d.lowerTranslation = -1;
    d.upperTranslation = 2;
    return b3CreatePrismaticJoint(world, &d);
  }
  case 5: {
    auto d = b3DefaultRevoluteJointDef();
    d.base = base;
    d.enableLimit = config & 1;
    d.lowerAngle = -.4f;
    d.upperAngle = .7f;
    return b3CreateRevoluteJoint(world, &d);
  }
  case 6: {
    auto d = b3DefaultSphericalJointDef();
    d.base = base;
    d.enableConeLimit = config & 1;
    d.enableTwistLimit = config & 2;
    d.coneAngle = .3f;
    d.lowerTwistAngle = -.4f;
    d.upperTwistAngle = .7f;
    return b3CreateSphericalJoint(world, &d);
  }
  case 7: {
    auto d = b3DefaultWeldJointDef();
    d.base = base;
    d.linearHertz = config & 1 ? 2 : 0;
    d.angularHertz = config & 2 ? 3 : 0;
    return b3CreateWeldJoint(world, &d);
  }
  default: {
    auto d = b3DefaultWheelJointDef();
    d.base = base;
    d.enableSuspensionLimit = config & 1;
    d.lowerSuspensionLimit = -1;
    d.upperSuspensionLimit = 2;
    return b3CreateWheelJoint(world, &d);
  }
  }
}
int main() {
  for (kind = 0; kind < 9; ++kind) {
    auto wd = b3DefaultWorldDef();
    wd.gravity = {0, 0, 0};
    auto w = b3CreateWorld(&wd);
    world = w;
    auto bd = b3DefaultBodyDef();
    auto a = b3CreateBody(w, &bd);
    bd.type = b3_dynamicBody;
    auto b = b3CreateBody(w, &bd);
    auto sd = b3DefaultShapeDef();
    b3Sphere sphere = {{.2f, -.1f, .3f}, .5f};
    b3CreateSphereShape(b, &sd, &sphere);
    for (config = 0; config < 8; ++config) {
      auto base = b3DefaultWeldJointDef().base;
      base.bodyIdA = a;
      base.bodyIdB = b;
      if (config & 4) {
        base.localFrameA.p = {.3f, .5f, -.2f};
        base.localFrameB.p = {-.7f, .2f, .1f};
        base.localFrameA.q = b3MakeQuatFromAxisAngle({0, 1, 0}, .8f);
        base.localFrameB.q = b3MakeQuatFromAxisAngle({1, 0, 0}, -.9f);
      }
      auto j = create(base);
      assert(b3Joint_IsValid(j));
      for (pose = 0; pose < 16; ++pose) {
        auto qa = b3MakeQuatFromAxisAngle({0, 0, 1}, pose % 2 ? 1.3f : 0.f);
        auto qb =
            b3MulQuat(b3MakeQuatFromAxisAngle({1, 0, 0}, .11f * pose),
                      b3MakeQuatFromAxisAngle({0, 0, 1}, -.9f + .2f * pose));
        if (pose & 4) {
          qb.v = b3Neg(qb.v);
          qb.s = -qb.s;
        }
        b3Body_SetTransform(a, {0, 0, 0}, qa);
        b3Body_SetTransform(
            b,
            {float(pose % 5 - 2) * 2.f, float(pose % 3 - 1), float(pose % 4)},
            qb);
        inspect(j);
      }
      switch (kind) {
      case 0:
        b3DistanceJoint_EnableSpring(j, true);
        b3DistanceJoint_EnableLimit(j, true);
        b3DistanceJoint_SetLengthRange(j, .5f, 1.f);
        break;
      case 4:
        b3PrismaticJoint_EnableLimit(j, true);
        b3PrismaticJoint_SetLimits(j, -.2f, .3f);
        break;
      case 5:
        b3RevoluteJoint_EnableLimit(j, true);
        b3RevoluteJoint_SetLimits(j, -.2f, .3f);
        break;
      case 6:
        b3SphericalJoint_EnableConeLimit(j, true);
        b3SphericalJoint_SetConeLimit(j, .1f);
        b3SphericalJoint_EnableTwistLimit(j, true);
        b3SphericalJoint_SetTwistLimits(j, -.2f, .3f);
        break;
      case 7:
        b3WeldJoint_SetLinearHertz(j, 1.f);
        b3WeldJoint_SetAngularHertz(j, 1.f);
        break;
      case 8:
        b3WheelJoint_EnableSuspensionLimit(j, true);
        b3WheelJoint_SetSuspensionLimits(j, -.2f, .3f);
        break;
      }
      inspect(j);
      b3Joint_SetLocalFrameA(j, {{-.2f, .4f, .6f}, b3Quat_identity});
      inspect(j);
      b3Joint_SetLocalFrameB(j, {{.1f, -.3f, .2f}, b3Quat_identity});
      inspect(j);
      b3DestroyJoint(j, false);
      assert(!b3Joint_IsValid(j));
      assert(b3Joint_GetLinearSeparation(j) == 0);
      assert(b3Joint_GetAngularSeparation(j) == 0);
    }
    config = 0;
    auto base = b3DefaultWeldJointDef().base;
    base.bodyIdA = a;
    base.bodyIdB = b;
    b3Body_SetTransform(a, {0, 0, 0}, b3Quat_identity);
    b3Body_SetTransform(b, {kind == 0 ? 2.f : 0.f, 0, 0}, b3Quat_identity);
    auto j = create(base);
    b3Body_SetLinearVelocity(b, {.1f, .2f, -.1f});
    for (pose = 0; pose < 5; ++pose) {
      b3World_Step(w, 1.f / 60, 4);
      inspect(j);
    }
    b3Body_SetAwake(b, false);
    inspect(j);
    if (kind == 7) {
      // Prove combined getters are GPU-backed, not CPU passthroughs.
      float linear = b3Joint_GetLinearSeparation(j);
      float angular = b3Joint_GetAngularSeparation(j);
      cpu_b3Body_SetTransform(both_cpu_body(b), {10, 3, 0},
                              b3MakeQuatFromAxisAngle({1, 0, 0}, 1.f));
      assert(b3Joint_GetLinearSeparation(j) == linear);
      assert(b3Joint_GetAngularSeparation(j) == angular);
      assert(cpu_b3Joint_GetLinearSeparation(both_cpu_joint(j)) > 1.f);
      assert(cpu_b3Joint_GetAngularSeparation(both_cpu_joint(j)) > .5f);
    }
    b3DestroyWorld(w);
  }
  printf("joint separation: %d independent CPU/GPU checks passed across nine "
         "joint types; wheel angular unsupported upstream\n",
         checks);
}
