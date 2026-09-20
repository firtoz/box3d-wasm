#include "box3d/box3d.h"
extern "C" {
#include "both_ids.h"
b3Vec3 cpu_b3Joint_GetConstraintForce(b3JointId);
b3Vec3 cpu_b3Joint_GetConstraintTorque(b3JointId);
bool cpu_b3Body_IsAwake(b3BodyId);
void cpu_b3World_Step(b3WorldId, float, int);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
static int kind, config, step, checks;
static b3WorldId world;
static bool sawForce[9], sawTorque[9];
static void near(b3Vec3 g, b3Vec3 c, const char *label) {
  for (int i = 0; i < 3; ++i) {
    float a = (&g.x)[i], b = (&c.x)[i];
    if (!(std::isfinite(a) && std::isfinite(b) &&
          fabsf(a - b) <= 1e-5f * fmaxf(1, fmaxf(fabsf(a), fabsf(b))))) {
      fprintf(stderr, "kind=%d config=%d step=%d %s[%d] GPU=%.9g CPU=%.9g\n",
              kind, config, step, label, i, a, b);
      abort();
    }
    ++checks;
  }
}
static void inspect(b3JointId j) {
  auto f = b3Joint_GetConstraintForce(j), t = b3Joint_GetConstraintTorque(j);
  sawForce[kind] |= b3Length(f) > 1e-4f;
  sawTorque[kind] |= b3Length(t) > 1e-4f;
  near(f, cpu_b3Joint_GetConstraintForce(both_cpu_joint(j)), "force");
  near(t, cpu_b3Joint_GetConstraintTorque(both_cpu_joint(j)), "torque");
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
    d.enableMotor = config >= 2;
    d.maxMotorForce = 2;
    d.motorSpeed = .4f;
    return b3CreateDistanceJoint(world, &d);
  }
  case 1: {
    auto d = b3DefaultMotorJointDef();
    d.base = base;
    if (config >= 2) {
      d.maxVelocityForce = 2;
      d.maxVelocityTorque = .2f;
      d.linearVelocity = {.2f, .1f, 0};
      d.angularVelocity = {.3f, .1f, .2f};
    }
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
    if (config == 2)
      d.hertz = 0;
    if (config == 3)
      d.maxTorque = .2f;
    return b3CreateParallelJoint(world, &d);
  }
  case 4: {
    auto d = b3DefaultPrismaticJointDef();
    if (config == 2 || (config == 3 && getenv("GPU_REACTION_OFFSET_STRESS"))) {
      base.localFrameA.p = {.5f, 0, 0};
      base.localFrameB.p = base.localFrameA.p;
    }
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
    if (config >= 2) {
      d.enableMotor = true;
      d.maxMotorTorque = .1f;
      d.motorSpeed = 1;
    }
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
    if (config >= 2) {
      d.enableMotor = true;
      d.maxMotorTorque = .1f;
      d.motorVelocity = {.3f, .2f, .4f};
    }
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
    if (config >= 2) {
      d.enableSpinMotor = true;
      d.maxSpinTorque = .1f;
      d.spinSpeed = .2f;
    }
    return b3CreateWheelJoint(world, &d);
  }
  }
}
int main() {
  for (kind = 0; kind < 9; ++kind)
    for (config = 0; config < 4; ++config) {
      if (getenv("GPU_REACTION_KIND") && kind != atoi(getenv("GPU_REACTION_KIND"))) continue;
      auto wd = b3DefaultWorldDef();
      world = b3CreateWorld(&wd);
      auto bd = b3DefaultBodyDef();
      auto a = b3CreateBody(world, &bd);
      bd.type = b3_dynamicBody;
      bd.position = kind == 0 ? b3Pos{0, 2, 0} : b3Pos{0, 0, 0};
      auto b = b3CreateBody(world, &bd);
      auto sd = b3DefaultShapeDef();
      sd.density = getenv("GPU_REACTION_DENSITY")
                       ? strtof(getenv("GPU_REACTION_DENSITY"), nullptr)
                       : 1.f; // Unit-density fixtures keep absolute reaction
                              // units comparable across joint types.
      b3Sphere sphere = {{0, 0, 0}, .5f};
      b3CreateSphereShape(b, &sd, &sphere);
      auto base = b3DefaultWeldJointDef().base;
      base.bodyIdA = a;
      base.bodyIdB = b;
      if (config & 1) {
        auto q = b3MakeQuatFromAxisAngle({0, 1, 0}, .4f);
        base.localFrameA.q = q;
        base.localFrameB.q = q;
      }
      auto j = create(base);
      step = -1;
      inspect(j);
      b3Body_SetAngularVelocity(b, {.1f, .2f, .3f});
      for (step = 0; step < 8; ++step) {
        // Optional probe for the existing pre-step versus substep torque integration gap.
        if (getenv("GPU_REACTION_APPLIED_TORQUE"))
          b3Body_ApplyTorque(b, {.03f, .02f, .01f}, true);
        b3World_Step(world, step < 4 ? 1.f / 60 : 1.f / 120,
                     config >= 2 ? 3 : 4);
        inspect(j);
      }
      b3Body_SetAwake(b, false);
      inspect(j);
      b3Joint_SetLocalFrameA(
          j, {{.1f, .2f, .3f}, b3MakeQuatFromAxisAngle({1, 0, 0}, .3f)});
      inspect(j);
      if (kind == 0) {
        b3DistanceJoint_SetLength(j, 2.f);
        inspect(j);
        b3DistanceJoint_EnableMotor(j, false);
        inspect(j);
      }
      if (kind == 7 && config == 0) {
        auto before = b3Joint_GetConstraintForce(j);
        assert(b3Length(before) > 1.f);
        cpu_b3World_Step(both_cpu_world(world), 0, 4);
        assert(b3Length(cpu_b3Joint_GetConstraintForce(both_cpu_joint(j))) ==
               0);
        auto after = b3Joint_GetConstraintForce(j);
        assert(before.x == after.x && before.y == after.y &&
               before.z == after.z);
      }
      b3World_Step(world, 0, 4);
      inspect(j);
      // Existing GPU parameter setters wake bodies; explicitly restore the shared sleep condition.
      b3Body_SetAwake(b, false);
      step = 85;
      b3World_Step(world, 1.f / 30, 2);
      assert(!b3Body_IsAwake(b) && !cpu_b3Body_IsAwake(both_cpu_body(b)));
      inspect(j);
      b3DestroyJoint(j, false);
      auto f = b3Joint_GetConstraintForce(j),
           t = b3Joint_GetConstraintTorque(j);
      assert(b3Length(f) == 0 && b3Length(t) == 0);
      b3DestroyWorld(world);
    }
  for (int k = 0; k < 9; ++k) {
    if (getenv("GPU_REACTION_KIND") && k != atoi(getenv("GPU_REACTION_KIND"))) continue;
    if (k != 2 && k != 3)
      assert(sawForce[k]);
    if (k != 0 && k != 2 && !sawTorque[k]) {
      fprintf(stderr, "kind=%d had no nonzero torque case\n", k);
      abort();
    }
  }
  printf("joint reaction: %d independent CPU/GPU scalar comparisons passed\n",
         checks);
}
