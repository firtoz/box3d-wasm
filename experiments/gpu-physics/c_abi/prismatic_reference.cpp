// Same fixture linked against either real Box3D or the GPU C ABI.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cstdio>
#ifdef GPU_REFERENCE
extern "C" void gpu_b3_world_wait(b3WorldId);
#endif
int main() {
    for (int mode = 0; mode < 5; ++mode) {
        b3WorldDef wd = b3DefaultWorldDef();
        wd.gravity = {0, 0, 0};
        wd.enableSleep = false;
        wd.enableContinuous = false;
        b3WorldId world = b3CreateWorld(&wd);
        b3BodyDef ad = b3DefaultBodyDef();
        if (mode > 0) {
            ad.type = b3_dynamicBody;
            ad.position = {-1, 0, 0};
            ad.angularVelocity = {0.2f, -0.1f, 0.3f};
        }
        b3BodyId a = b3CreateBody(world, &ad);
        b3ShapeDef sd = b3DefaultShapeDef();
        b3BoxHull ah = b3MakeBoxHull(0.3f, 0.4f, 0.5f);
        b3CreateHullShape(a, &sd, &ah.base);
        b3BodyDef bd = b3DefaultBodyDef();
        bd.type = b3_dynamicBody;
        bd.linearVelocity = {0, 3, 4};
        if (mode > 0) { bd.position = {1, 0, 0}; }
        b3BodyId b = b3CreateBody(world, &bd);
        b3BoxHull bh = b3MakeBoxHull(0.2f, 0.3f, 0.4f);
        b3CreateHullShape(b, &sd, &bh.base);
        b3PrismaticJointDef jd = b3DefaultPrismaticJointDef();
        jd.base.bodyIdA = a;
        jd.base.bodyIdB = b;
        if (mode > 0) {
            jd.base.localFrameA.p = {1, 0.3f, 0.2f};
            jd.base.localFrameB.p = {-1, 0.3f, 0.2f};
            jd.base.localFrameA.q = {{0, 0, 0.38268343f}, 0.92387953f};
            jd.base.localFrameB.q = jd.base.localFrameA.q;
        }
        if (mode == 2) { jd.enableMotor = true; jd.motorSpeed = 0.7f; jd.maxMotorForce = 10; }
        if (mode == 3) {
            jd.enableSpring = true; jd.hertz = 2; jd.dampingRatio = 0.7f; jd.targetTranslation = 0.4f;
        }
        if (mode == 4) {
            jd.enableMotor = true; jd.motorSpeed = 2; jd.maxMotorForce = 20;
            jd.enableLimit = true; jd.lowerTranslation = -0.1f; jd.upperTranslation = 0.1f;
        }
        b3CreatePrismaticJoint(world, &jd);
        for (int step = 1; step <= 120; ++step) {
            b3World_Step(world, 1.0f/60, mode == 0 ? 1 : 4);
#ifdef GPU_REFERENCE
            gpu_b3_world_wait(world);
#endif
            b3BodyId ids[] = {a, b};
            for (int index = 0; index < 2; ++index) {
                b3Pos p = b3Body_GetPosition(ids[index]);
                b3Quat q = b3Body_GetRotation(ids[index]);
                b3Vec3 v = b3Body_GetLinearVelocity(ids[index]);
                b3Vec3 w = b3Body_GetAngularVelocity(ids[index]);
                std::printf("%d %d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",
                    mode, step, index, double(p.x), double(p.y), double(p.z),
                    q.v.x, q.v.y, q.v.z, q.s, v.x, v.y, v.z, w.x, w.y, w.z);
            }
        }
        b3DestroyWorld(world);
    }
}
