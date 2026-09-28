#pragma once
#include "box3d/box3d.h"
#include "human.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

// Audit definitions through the engine getters, not just the shared builder.
static void dumpHumanJointSetup(const std::vector<const Human*>& humans)
{
    std::vector<b3BodyId> bodies;
    for (const auto* human : humans)
        for (const auto& bone : human->bones) bodies.push_back(bone.bodyId);
    auto bodyIndex = [&](b3BodyId id) {
        for (int i = 0; i < int(bodies.size()); ++i)
            if (B3_ID_EQUALS(id, bodies[i])) return i;
        std::abort();
    };
    auto value = [](const char* name, float v) { std::fprintf(stderr, " %s=%.9g", name, v); };
    int humanIndex = 0, jointCount = 0;
    for (const auto* humanPointer : humans)
        {
            const auto& human = *humanPointer;
            auto emit = [&](int slot, b3JointId joint) {
                if (B3_IS_NULL(joint)) return;
                ++jointCount;
                const auto type = b3Joint_GetType(joint);
                std::fprintf(stderr, "joint-setup %d %d %d %d %d", humanIndex, slot, int(type),
                    bodyIndex(b3Joint_GetBodyA(joint)), bodyIndex(b3Joint_GetBodyB(joint)));
                value("collide", b3Joint_GetCollideConnected(joint));
                const auto a = b3Joint_GetLocalFrameA(joint), b = b3Joint_GetLocalFrameB(joint);
                const char* names[] = {"ax","ay","az","aqx","aqy","aqz","aqw",
                    "bx","by","bz","bqx","bqy","bqz","bqw"};
                const float frames[] = {a.p.x,a.p.y,a.p.z,a.q.v.x,a.q.v.y,a.q.v.z,a.q.s,
                    b.p.x,b.p.y,b.p.z,b.q.v.x,b.q.v.y,b.q.v.z,b.q.s};
                for (int i = 0; i < 14; ++i) value(names[i], frames[i]);
                float hertz, damping;
                b3Joint_GetConstraintTuning(joint, &hertz, &damping);
                value("constraint_hertz", hertz); value("constraint_damping", damping);
                value("force_threshold", b3Joint_GetForceThreshold(joint));
                value("torque_threshold", b3Joint_GetTorqueThreshold(joint));
                if (type == b3_revoluteJoint)
                {
                    value("spring", b3RevoluteJoint_IsSpringEnabled(joint));
                    value("spring_hertz", b3RevoluteJoint_GetSpringHertz(joint));
                    value("spring_damping", b3RevoluteJoint_GetSpringDampingRatio(joint));
                    value("target", b3RevoluteJoint_GetTargetAngle(joint));
                    value("limit", b3RevoluteJoint_IsLimitEnabled(joint));
                    value("lower", b3RevoluteJoint_GetLowerLimit(joint));
                    value("upper", b3RevoluteJoint_GetUpperLimit(joint));
                    value("motor", b3RevoluteJoint_IsMotorEnabled(joint));
                    value("motor_speed", b3RevoluteJoint_GetMotorSpeed(joint));
                    value("motor_torque", b3RevoluteJoint_GetMaxMotorTorque(joint));
                }
                else if (type == b3_sphericalJoint)
                {
                    value("spring", b3SphericalJoint_IsSpringEnabled(joint));
                    value("spring_hertz", b3SphericalJoint_GetSpringHertz(joint));
                    value("spring_damping", b3SphericalJoint_GetSpringDampingRatio(joint));
                    const auto q = b3SphericalJoint_GetTargetRotation(joint);
                    value("target_x",q.v.x); value("target_y",q.v.y); value("target_z",q.v.z); value("target_w",q.s);
                    value("cone", b3SphericalJoint_IsConeLimitEnabled(joint));
                    value("cone_angle", b3SphericalJoint_GetConeLimit(joint));
                    value("twist", b3SphericalJoint_IsTwistLimitEnabled(joint));
                    value("lower", b3SphericalJoint_GetLowerTwistLimit(joint));
                    value("upper", b3SphericalJoint_GetUpperTwistLimit(joint));
                    value("motor", b3SphericalJoint_IsMotorEnabled(joint));
                    const auto v = b3SphericalJoint_GetMotorVelocity(joint);
                    value("motor_x",v.x); value("motor_y",v.y); value("motor_z",v.z);
                    value("motor_torque", b3SphericalJoint_GetMaxMotorTorque(joint));
                }
                std::fputc('\n', stderr);
            };
            for (int i = 0; i < bone_count; ++i) emit(i, human.bones[i].jointId);
            for (int i = 0; i < human.filterJointCount; ++i) emit(bone_count+i, human.filterJoints[i]);
            ++humanIndex;
        }
    int incidences = 0;
    for (auto body : bodies) incidences += b3Body_GetJointCount(body);
    std::fprintf(stderr, "joint-setup-count %zu %d %d\n", bodies.size(), jointCount, incidences);
}
