// Exercises public C symbols and, in dual mode, verifies each world's own state.
#include "box3d/box3d.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
extern "C" void gpu_b3_distance_set_length(b3JointId, float);
extern "C" float cpu_b3DistanceJoint_GetLength(b3JointId);
extern "C" b3WorldId both_cpu_world(b3WorldId);
extern "C" b3BodyId both_cpu_body(b3BodyId);
extern "C" b3JointId both_cpu_joint(b3JointId);
extern "C" float cpu_b3Body_GetSleepThreshold(b3BodyId);
extern "C" bool cpu_b3Body_IsSleepEnabled(b3BodyId);
extern "C" const char* cpu_b3Body_GetName(b3BodyId);
extern "C" void* cpu_b3World_GetUserData(b3WorldId);
extern "C" b3Transform cpu_b3Joint_GetLocalFrameA(b3JointId);
#endif
int main() {
    auto wd = b3DefaultWorldDef();
    wd.gravity = {0,0,0}; wd.userData = (void*)0x1234;
    wd.contactHertz=12; wd.contactDampingRatio=0.75f; wd.contactSpeed=2;
    auto w = b3CreateWorld(&wd);
    assert(b3World_GetUserData(w)==wd.userData);
    b3World_SetUserData(w,(void*)0x5678);
    assert(b3World_GetUserData(w)==(void*)0x5678);
    auto bd=b3DefaultBodyDef(); bd.type=b3_dynamicBody;
    bd.sleepThreshold=0.123f; bd.name="first";
    bd.linearDamping=0.25f; bd.angularDamping=0.75f; bd.gravityScale=2;
    auto a=b3CreateBody(w,&bd);
    assert(b3Body_GetLinearDamping(a)==0.25f);
    assert(b3Body_GetAngularDamping(a)==0.75f);
    assert(b3Body_GetGravityScale(a)==2);
    assert(b3Body_IsAwake(a));
    assert(b3Body_GetSleepThreshold(a)==0.123f);
    assert(strcmp(b3Body_GetName(a),"first")==0);
    char name[]="copied"; b3Body_SetName(a,name); name[0]='X';
    assert(strcmp(b3Body_GetName(a),"copied")==0);
    b3Body_SetSleepThreshold(a,0.234f);
    b3Body_EnableSleep(a,false);
    assert(!b3Body_IsSleepEnabled(a));
    auto sd=b3DefaultShapeDef(); auto sphere=b3Sphere{{0,0,0},0.5f};
    auto sa=b3CreateSphereShape(a,&sd,&sphere);
    auto sm=b3Shape_ComputeMassData(sa);
    auto expected=b3ComputeSphereMass(&sphere,sd.density);
    assert(fabsf(sm.mass-expected.mass)<0.001f);
    assert(fabsf(sm.inertia.cx.x-expected.inertia.cx.x)<0.001f);
    bd.position={10,0,0}; auto b=b3CreateBody(w,&bd);
    auto sb=b3CreateSphereShape(b,&sd,&sphere);
    b3Body_EnableHitEvents(a,true);
    assert(b3Shape_AreHitEventsEnabled(sa)); assert(!b3Shape_AreHitEventsEnabled(sb));
    auto jd=b3DefaultRevoluteJointDef(); jd.base.bodyIdA=a; jd.base.bodyIdB=b;
    auto joint=b3CreateRevoluteJoint(w,&jd);
    b3Transform frame={{0.2f,0.3f,0.4f},b3Quat_identity};
    b3Joint_SetLocalFrameA(joint,frame);
    auto actual=b3Joint_GetLocalFrameA(joint);
    assert(actual.p.x==frame.p.x && actual.p.y==frame.p.y && actual.p.z==frame.p.z);
    b3Joint_SetLocalFrameB(joint,frame);
    assert(b3Joint_GetLocalFrameB(joint).p.z==frame.p.z);
#ifdef GPU_API_DUAL
    assert(cpu_b3World_GetUserData(both_cpu_world(w))==(void*)0x5678);
    assert(cpu_b3Body_GetSleepThreshold(both_cpu_body(a))==0.234f);
    assert(!cpu_b3Body_IsSleepEnabled(both_cpu_body(a)));
    assert(strcmp(cpu_b3Body_GetName(both_cpu_body(a)),"copied")==0);
    assert(cpu_b3Joint_GetLocalFrameA(both_cpu_joint(joint)).p.x==frame.p.x);
#endif
#ifdef GPU_API_DUAL
    auto dd=b3DefaultDistanceJointDef(); dd.base.bodyIdA=a; dd.base.bodyIdB=b; dd.length=2;
    auto distance=b3CreateDistanceJoint(w,&dd);
    gpu_b3_distance_set_length(distance,3);
    assert(b3DistanceJoint_GetLength(distance)==3);
    assert(cpu_b3DistanceJoint_GetLength(both_cpu_joint(distance))==2);
#endif
    auto kd=b3DefaultBodyDef(); kd.type=b3_kinematicBody;
    auto k=b3CreateBody(w,&kd);
    b3WorldTransform target={{0,2,0},b3Quat_identity};
    b3Body_SetTargetTransform(k,target,0.5f,true);
    assert(fabs(b3Body_GetPosition(k).y)<1e-6); // target is a velocity request, not a teleport
    assert(fabsf(b3Body_GetLinearVelocity(k).y-4.0f)<1e-5f);
    b3DestroyWorld(w);
    assert(!b3SaveRecordingToFile(nullptr, "recording-must-not-be-created.b3rec"));
    puts("C API settings: pass");
}
