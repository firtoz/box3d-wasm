// Reduced Gear Lift: upstream chain/gate dimensions and tuning, no gears/terrain.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#ifdef GPU_REFERENCE
extern "C" void gpu_b3_world_wait(b3WorldId);
#endif
struct Hinge { b3BodyId a, b; };
int main(int argc, char** argv) {
    float kick = argc > 1 ? std::strtof(argv[1], nullptr) : 0;
    bool contacts = argc > 2 && std::atoi(argv[2]) != 0;
    b3WorldDef wd = b3DefaultWorldDef();
    wd.enableSleep = false;
    wd.enableContinuous = false;
    b3WorldId world = b3CreateWorld(&wd);
    b3BodyDef ad = b3DefaultBodyDef();
    b3BodyId ground = b3CreateBody(world, &ad);
    b3BodyId last[2];
    std::vector<b3BodyId> bodies;
    std::vector<Hinge> hinges;
    b3ShapeDef sd = b3DefaultShapeDef();
    if (!contacts) sd.filter.maskBits = 0;
    for (int side = 0; side < 2; ++side) {
        float z = side == 0 ? -1.5f : 1.5f;
        float y = 10.75f - 0.07f;
        b3BodyId prev = ground;
        for (int link = 0; link < 40; ++link) {
            b3BodyDef bd = b3DefaultBodyDef();
            bd.type = b3_dynamicBody;
            bd.position = {0, y, z};
            b3BodyId body = b3CreateBody(world, &bd);
            b3Capsule cap = {{0,-0.07f,0},{0,0.07f,0},0.05f};
            b3CreateCapsuleShape(body, &sd, &cap);
            b3Pos pivot = {0,y+0.07f,z};
            b3RevoluteJointDef jd = b3DefaultRevoluteJointDef();
            jd.base.bodyIdA = prev; jd.base.bodyIdB = body;
            jd.base.localFrameA.p = b3Body_GetLocalPoint(prev,pivot);
            jd.base.localFrameB.p = b3Body_GetLocalPoint(body,pivot);
            jd.enableMotor = true; jd.maxMotorTorque = 0.05f;
            b3CreateRevoluteJoint(world,&jd);
            hinges.push_back({prev,body}); bodies.push_back(body);
            prev=body; y-=0.14f;
        }
        last[side]=prev;
    }
    b3BodyDef dd = b3DefaultBodyDef(); dd.type=b3_dynamicBody;
    dd.position={0,10.75f-(2*40*0.07f+1.5f),0};
    b3BodyId door=b3CreateBody(world,&dd); bodies.push_back(door);
    sd.density*=0.5f; sd.baseMaterial.friction=0.1f;
    b3BoxHull box=b3MakeBoxHull(0.05f,1.5f,1.95f);
    b3CreateHullShape(door,&sd,&box.base);
    for (int side=0; side<2; ++side) {
        float z=side==0?-1.5f:1.5f;
        b3RevoluteJointDef jd=b3DefaultRevoluteJointDef();
        jd.base.bodyIdA=last[side]; jd.base.bodyIdB=door;
        jd.base.localFrameA.p=b3Body_GetLocalPoint(last[side],{0,dd.position.y+1.5f,z});
        jd.base.localFrameB.p={0,1.5f,z};
        jd.enableMotor=true; jd.maxMotorTorque=50;
        b3CreateRevoluteJoint(world,&jd); hinges.push_back({last[side],door});
    }
    b3PrismaticJointDef pd=b3DefaultPrismaticJointDef();
    pd.base.bodyIdA=ground; pd.base.bodyIdB=door;
    pd.base.localFrameA.p={0,float(dd.position.y),0};
    pd.base.localFrameA.q=b3ComputeQuatBetweenUnitVectors(b3Vec3_axisX,b3Vec3_axisY);
    pd.base.localFrameB.q=pd.base.localFrameA.q;
    pd.base.collideConnected=true; pd.enableMotor=true; pd.maxMotorForce=200;
    b3CreatePrismaticJoint(world,&pd);
    for (int step=0; step<=120; ++step) {
        if (step==2) b3Body_SetLinearVelocity(door,{0,0,kick});
        if (step>0) b3World_Step(world,1.0f/60,4);
#ifdef GPU_REFERENCE
        gpu_b3_world_wait(world);
#endif
        float maxError=0; int worst=-1;
        for (int i=0;i<int(hinges.size());++i) {
            b3Quat q=b3InvMulQuat(b3Body_GetRotation(hinges[i].a),b3Body_GetRotation(hinges[i].b));
            float e=2*std::atan2(std::sqrt(q.v.x*q.v.x+q.v.y*q.v.y),std::sqrt(q.v.z*q.v.z+q.s*q.s));
            if(e>maxError) { maxError=e; worst=i; }
        }
        std::printf("{\"step\":%d,\"angular_error\":%.9g,\"joint\":%d,\"bodies\":[",step,maxError,worst);
        for(int i=0;i<int(bodies.size());++i) {
            auto id=bodies[i]; b3Pos p=b3Body_GetPosition(id); b3Quat q=b3Body_GetRotation(id);
            b3Vec3 v=b3Body_GetLinearVelocity(id), w=b3Body_GetAngularVelocity(id);
            std::printf("%s[%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g]",i?",":"",
                double(p.x),double(p.y),double(p.z),q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z,b3Body_GetMass(id));
        }
        std::puts("]}");
    }
    b3DestroyWorld(world);
}
