#include "box3d/box3d.h"
#include "determinism.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <initializer_list>
extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));

static void dump(int frame, int index, b3BodyId body, b3JointId joint)
{
    auto p = b3Body_GetPosition(body);
    auto q = b3Body_GetRotation(body);
    auto v = b3Body_GetLinearVelocity(body);
    auto w = b3Body_GetAngularVelocity(body);
    float separation = B3_IS_NON_NULL(joint) ? b3Joint_GetLinearSeparation(joint) : 0;
    std::printf("%d %d", frame, index);
    for (float x : {p.x,p.y,p.z,q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z,separation})
        std::printf(" %.9g", x);
    std::puts("");
}
int main(int argc, char** argv)
{
    bool ragdolls = argc > 1 && std::strcmp(argv[1], "ragdolls") == 0;
    int steps = argc > 2 ? std::atoi(argv[2]) : 300;
    auto wd = b3DefaultWorldDef();
    if (!ragdolls) wd.gravity = b3Vec3_zero;
    auto world = b3CreateWorld(&wd);
    FallingRagdollData data = {};
    b3BodyId body = {};
    b3JointId joint = {};
    if (ragdolls) data = CreateFallingRagdolls(world);
    else
    {
        auto bd = b3DefaultBodyDef();
        auto ground = b3CreateBody(world, &bd);
        bd.type = b3_dynamicBody;
        if (argc > 1 && std::strstr(argv[1], "tilted") != nullptr)
            bd.rotation = b3MakeQuatFromAxisAngle(b3Vec3_axisX, 0.5f);
        body = b3CreateBody(world, &bd);
        auto sd = b3DefaultShapeDef();
        auto hull = b3MakeBoxHull(.5f,.5f,.5f);
        b3CreateHullShape(body,&sd,&hull.base);
        auto jd = b3DefaultSphericalJointDef();
        jd.base.bodyIdA = ground; jd.base.bodyIdB = body;
        jd.enableTwistLimit = true;
        jd.lowerTwistAngle = -.2f; jd.upperTwistAngle = .2f;
        joint = b3CreateSphericalJoint(world,&jd);
        b3Body_SetAngularVelocity(body,{0,0,argc > 1 && std::strstr(argv[1], "negative") ? -5.0f : 5.0f});
    }
    for (int frame = 0; frame <= steps; ++frame)
    {
        if (frame)
        {
            b3World_Step(world,1.f/60,4);
            if (b3_world_gpu_wait_with_mirror) b3_world_gpu_wait_with_mirror(world);
        }
        if (ragdolls)
        {
            int index=0;
            for (auto& group : data.groups)
                for (auto& human : group.humans)
                    for (auto& bone : human.bones)
                        dump(frame,index++,bone.bodyId,bone.jointId);
        }
        else dump(frame,0,body,joint);
    }
    if (ragdolls) DestroyFallingRagdolls(&data);
    b3DestroyWorld(world);
}
