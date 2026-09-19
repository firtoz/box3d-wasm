// SPDX-License-Identifier: MIT
// Body Type and Gyroscopic Torque setup copied from upstream sample_bodies.cpp.
// Headless fixture: camera calls are no-ops; physics definitions are unchanged.
#include "box3d/box3d.h"
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
std::vector<b3BodyId> bodies;
b3BodyId track(b3WorldId w, const b3BodyDef *d) {
  auto b = b3CreateBody(w, d);
  bodies.push_back(b);
  return b;
}
#define b3CreateBody track
struct Camera {
  void SetView(float, float, float, b3Pos) {}
};
struct SampleContext {
  bool restart = true;
};
struct Sample {
  b3WorldId m_worldId;
  SampleContext *m_context;
  Camera camera;
  Camera *m_camera = &camera;
  Sample(SampleContext *c) : m_context(c) {
    auto d = b3DefaultWorldDef();
    m_worldId = b3CreateWorld(&d);
  }
  b3BodyId AddGroundBox(float x) {
    auto d = b3DefaultBodyDef();
    d.position = {0, -1, 0};
    auto b = b3CreateBody(m_worldId, &d);
    auto h = b3MakeBoxHull(x, 1, x);
    auto sd = b3DefaultShapeDef();
    b3CreateHullShape(b, &sd, &h.base);
    return b;
  }
};
struct BodyType : Sample {
  b3BodyId m_attachmentId, m_secondAttachmentId, m_platformId,
      m_secondPayloadId, m_touchingBodyId, m_floatingBodyId;
  b3BodyType m_type;
  float m_speed;
  bool m_isEnabled;
  explicit BodyType(SampleContext *context) : Sample(context) {
    if (m_context->restart == false) {
      m_camera->SetView(0.0f, 30.0f, 30.0f, {0.0f, 1.5f, 0.0f});
    }

    m_type = b3_dynamicBody;
    m_isEnabled = true;

    b3BodyId groundId = AddGroundBox(20.0f);

    // Define attachment
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = b3_dynamicBody;
      bodyDef.position = {-2.0f, 3.0f, 0.0f};
      bodyDef.name = "attach1";
      m_attachmentId = b3CreateBody(m_worldId, &bodyDef);

      b3BoxHull box = b3MakeBoxHull(0.5f, 2.0f, 0.5f);
      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 1.0f;
      b3CreateHullShape(m_attachmentId, &shapeDef, &box.base);
    }

    // Define second attachment
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = m_type;
      bodyDef.isEnabled = m_isEnabled;
      bodyDef.position = {3.0f, 3.0f};
      bodyDef.name = "attach2";
      m_secondAttachmentId = b3CreateBody(m_worldId, &bodyDef);

      b3BoxHull box = b3MakeBoxHull(0.5f, 2.0f, 0.5f);
      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 1.0f;
      b3CreateHullShape(m_secondAttachmentId, &shapeDef, &box.base);
    }

    // Define platform
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = m_type;
      bodyDef.isEnabled = m_isEnabled;
      bodyDef.position = {-4.0f, 5.0f};
      bodyDef.name = "platform";
      m_platformId = b3CreateBody(m_worldId, &bodyDef);

      b3BoxHull box = b3MakeTransformedBoxHull(
          0.5f, 4.0f, 0.5f,
          {{4.0f, 0.0f, 0.0f},
           b3MakeQuatFromAxisAngle(b3Vec3_axisZ, 0.5f * B3_PI)});

      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 2.0f;
      b3CreateHullShape(m_platformId, &shapeDef, &box.base);

      b3RevoluteJointDef revoluteDef = b3DefaultRevoluteJointDef();
      b3Pos pivot = {-2.0f, 5.0f, 0.0f};
      revoluteDef.base.bodyIdA = m_attachmentId;
      revoluteDef.base.bodyIdB = m_platformId;
      revoluteDef.base.localFrameA.p =
          b3Body_GetLocalPoint(m_attachmentId, pivot);
      revoluteDef.base.localFrameB.p =
          b3Body_GetLocalPoint(m_platformId, pivot);
      revoluteDef.maxMotorTorque = 50.0f;
      revoluteDef.enableMotor = true;
      b3CreateRevoluteJoint(m_worldId, &revoluteDef);

      pivot = {3.0f, 5.0f};
      revoluteDef.base.bodyIdA = m_secondAttachmentId;
      revoluteDef.base.bodyIdB = m_platformId;
      revoluteDef.base.localFrameA.p =
          b3Body_GetLocalPoint(m_secondAttachmentId, pivot);
      revoluteDef.base.localFrameB.p =
          b3Body_GetLocalPoint(m_platformId, pivot);
      revoluteDef.maxMotorTorque = 50.0f;
      revoluteDef.enableMotor = true;
      b3CreateRevoluteJoint(m_worldId, &revoluteDef);

      b3PrismaticJointDef prismaticDef = b3DefaultPrismaticJointDef();
      b3Pos anchor = {0.0f, 5.0f, 0.0f};
      prismaticDef.base.bodyIdA = groundId;
      prismaticDef.base.bodyIdB = m_platformId;
      prismaticDef.base.localFrameA.p = b3Body_GetLocalPoint(groundId, anchor);
      prismaticDef.base.localFrameB.p =
          b3Body_GetLocalPoint(m_platformId, anchor);
      prismaticDef.maxMotorForce = 1000.0f;
      prismaticDef.motorSpeed = 0.0f;
      prismaticDef.enableMotor = true;
      prismaticDef.lowerTranslation = -10.0f;
      prismaticDef.upperTranslation = 10.0f;
      prismaticDef.enableLimit = true;

      b3CreatePrismaticJoint(m_worldId, &prismaticDef);

      m_speed = 3.0f;
    }

    // Create a payload
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = b3_dynamicBody;
      bodyDef.position = {-3.0f, 8.0f};
      bodyDef.name = "crate1";
      b3BodyId bodyId = b3CreateBody(m_worldId, &bodyDef);

      b3BoxHull box = b3MakeBoxHull(0.75f, 0.75f, 0.75f);

      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 2.0f;

      b3CreateHullShape(bodyId, &shapeDef, &box.base);
    }

    // Create a second payload
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = m_type;
      bodyDef.isEnabled = m_isEnabled;
      bodyDef.position = {2.0f, 8.0f};
      bodyDef.name = "crate2";
      m_secondPayloadId = b3CreateBody(m_worldId, &bodyDef);

      b3BoxHull box = b3MakeBoxHull(0.75f, 0.75f, 0.75f);

      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 2.0f;

      b3CreateHullShape(m_secondPayloadId, &shapeDef, &box.base);
    }

    // Create a separate body on the ground
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = m_type;
      bodyDef.isEnabled = m_isEnabled;
      bodyDef.position = {8.0f, 0.2f};
      bodyDef.name = "debris";
      m_touchingBodyId = b3CreateBody(m_worldId, &bodyDef);

      b3Capsule capsule = {{0.0f, 0.0f}, {1.0f, 0.0f}, 0.25f};

      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 2.0f;

      b3CreateCapsuleShape(m_touchingBodyId, &shapeDef, &capsule);
    }

    // Create a separate floating body
    {
      b3BodyDef bodyDef = b3DefaultBodyDef();
      bodyDef.type = m_type;
      bodyDef.isEnabled = m_isEnabled;
      bodyDef.position = {-8.0f, 12.0f};
      bodyDef.gravityScale = 0.0f;
      bodyDef.name = "floater";
      m_floatingBodyId = b3CreateBody(m_worldId, &bodyDef);

      b3Sphere sphere = {{0.0f, 0.5f, 0.0f}, 0.25f};

      b3ShapeDef shapeDef = b3DefaultShapeDef();
      shapeDef.density = 2.0f;

      b3CreateSphereShape(m_floatingBodyId, &shapeDef, &sphere);
    }
  }
};
struct GyroscopicTorque : Sample {
  b3BodyId m_bodyId;
  explicit GyroscopicTorque(SampleContext *context) : Sample(context) {
    if (context->restart == false) {
      m_camera->SetView(0.0f, 20.0f, 4.0f, {0.0f, 2.0f, 0.0f});
    }

    AddGroundBox(20.0f);

    b3BodyDef bodyDef = b3DefaultBodyDef();
    bodyDef.type = b3_dynamicBody;
    bodyDef.position = {0.0f, 2.0f, 0.0f};
    bodyDef.rotation = b3MakeQuatFromAxisAngle(b3Vec3_axisX, -0.5f * B3_PI);
    bodyDef.gravityScale = 0.0f;
    m_bodyId = b3CreateBody(m_worldId, &bodyDef);

    b3ShapeDef shapeDef = b3DefaultShapeDef();
    shapeDef.updateBodyMass = false;
    b3HullData *cylinder = b3CreateCylinder(0.6f, 0.15f, 0.0f, 32);
    b3BoxHull box = b3MakeBoxHull(1.0f, 0.05f, 0.1f);
    b3CreateHullShape(m_bodyId, &shapeDef, cylinder);
    b3CreateHullShape(m_bodyId, &shapeDef, &box.base);
    b3Body_ApplyMassFromShapes(m_bodyId);

    // Set the angular velocity after creating the shapes and the local center
    // of mass is fixed.
    b3Body_SetAngularVelocity(m_bodyId, {0.01f, 0.01f, 10.0f});

    b3DestroyHull(cylinder);
  }
};
extern "C" void gpu_b3_world_wait(b3WorldId) __attribute__((weak));
int main(int argc, char **argv) {
  if (argc != 3 ||
      (std::strcmp(argv[1], "body") && std::strcmp(argv[1], "body-callback") && std::strcmp(argv[1], "gyro") &&
       std::strcmp(argv[1], "gyro-mass")) ||
      std::atoi(argv[2]) < 1 || std::atoi(argv[2]) > 3600)
    return 64;
  SampleContext ctx;
  Sample *s = std::strncmp(argv[1], "body", 4) == 0
                  ? (Sample *)new BodyType(&ctx)
                  : (Sample *)new GyroscopicTorque(&ctx);
  auto w = s->m_worldId;
  int callbackCount = 0;
  if (std::strcmp(argv[1], "body-callback") == 0) {
    for (auto body : bodies) {
      std::vector<b3ShapeId> shapes(b3Body_GetShapeCount(body));
      int count = b3Body_GetShapes(body, shapes.data(), shapes.size());
      for (int i = 0; i < count; ++i)
        b3Shape_EnablePreSolveEvents(shapes[i], true);
    }
    b3World_SetPreSolveCallback(w,
        [](b3ShapeId, b3ShapeId, b3Pos, b3Vec3, void *context) {
          ++*static_cast<int *>(context);
          return true;
        }, &callbackCount);
  }
  if (std::strcmp(argv[1], "gyro-mass") == 0) {
    auto id = bodies.back();
    auto mass = b3Body_GetMassData(id);
    mass.mass *= 2.0f;
    mass.inertia = {{4.0f, 0.1f, 0.02f}, {0.1f, 12.0f, -0.03f}, {0.02f, -0.03f, 15.0f}};
    b3Body_SetMassData(id, mass);
    auto actual = b3Body_GetMassData(id);
    assert(actual.mass == mass.mass && actual.inertia.cx.x == 4.0f && actual.inertia.cy.x == 0.1f);
  }

  for (auto b : bodies) {
    auto m = b3Body_GetMassData(b);
    auto i = m.inertia;
    printf("mass %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
           b.index1, m.mass, m.center.x, m.center.y, m.center.z, i.cx.x, i.cy.y,
           i.cz.z, i.cx.y, i.cx.z, i.cy.z);
  }
  for (int step = 0; step <= atoi(argv[2]); step++) {
    if (step)
      b3World_Step(w, 1.f / 60, 4);
    if (gpu_b3_world_wait)
      gpu_b3_world_wait(w);
    for (auto b : bodies) {
      auto p = b3Body_GetPosition(b);
      auto q = b3Body_GetRotation(b);
      auto v = b3Body_GetLinearVelocity(b);
      auto a = b3Body_GetAngularVelocity(b);
      printf("state %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g "
             "%.9g %.9g %.9g\n",
             step, b.index1, p.x, p.y, p.z, q.v.x, q.v.y, q.v.z, q.s, v.x, v.y,
             v.z, a.x, a.y, a.z);
      if (step >= 35 && step <= 39) {
        std::vector<b3ContactData> c(b3Body_GetContactCapacity(b));
        int n = b3Body_GetContactData(b, c.data(), c.size());
        for (int k = 0; k < n; k++) {
          if (!c[k].manifoldCount)
            continue;
          auto &m = c[k].manifolds[0];
          printf("contact %d %d %d %d n %.9g %.9g %.9g count %d\n", step,
                 b.index1, c[k].shapeIdA.index1, c[k].shapeIdB.index1,
                 m.normal.x, m.normal.y, m.normal.z, m.pointCount);
          for (int j = 0; j < m.pointCount; j++) {
            auto &p = m.points[j];
            printf("point %.9g impulse %.9g\n", p.separation, p.normalImpulse);
          }
        }
      }
    }
  }
  if (std::strcmp(argv[1], "body-callback") == 0) {
    assert(callbackCount > 0);
    printf("callbacks %d\n", callbackCount);
  }
  b3DestroyWorld(w);
}
