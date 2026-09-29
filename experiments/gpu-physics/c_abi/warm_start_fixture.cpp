// Tiny contact/joint fixtures shared by baseline timing and CPU/GPU verification.
#include "box3d/box3d.h"
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <initializer_list>
#ifdef GPU_API_DUAL
extern "C" {
b3BodyId both_cpu_body(b3BodyId);
b3WorldId both_cpu_world(b3WorldId);
b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId);
b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId);
b3WorldTransform cpu_b3Body_GetTransform(b3BodyId);
bool cpu_b3World_IsWarmStartingEnabled(b3WorldId);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#endif
static void check_body(b3BodyId b, int step, const char* scene) {
    auto t=b3Body_GetTransform(b);auto v=b3Body_GetLinearVelocity(b);auto omega=b3Body_GetAngularVelocity(b);
    assert(std::isfinite(t.p.y) && std::isfinite(v.y));
#ifdef GPU_API_DUAL
    auto c=both_cpu_body(b);auto ct=cpu_b3Body_GetTransform(c);auto cv=cpu_b3Body_GetLinearVelocity(c);auto cw=cpu_b3Body_GetAngularVelocity(c);
    double g[]={t.p.x,t.p.y,t.p.z,t.q.v.x,t.q.v.y,t.q.v.z,t.q.s,v.x,v.y,v.z,omega.x,omega.y,omega.z};
    double ref[]={ct.p.x,ct.p.y,ct.p.z,ct.q.v.x,ct.q.v.y,ct.q.v.z,ct.q.s,cv.x,cv.y,cv.z,cw.x,cw.y,cw.z};
    for(int i=0;i<13;i++) if(!std::isfinite(g[i]) || std::abs(g[i]-ref[i])>1e-5) {
        fprintf(stderr,"%s step=%d lane=%d GPU=%.9g CPU=%.9g\n",scene,step,i,g[i],ref[i]);std::abort();
    }
#endif
}
int main(int argc,char** argv) {
    bool timing=argc>1 && !strcmp(argv[1],"--timing");
    for(bool joint : {false,true}) for(int substeps : {1,4}) {
        auto wd=b3DefaultWorldDef();wd.enableSleep=false;wd.enableContinuous=false;
        auto w=b3CreateWorld(&wd); auto bd=b3DefaultBodyDef();auto ground=b3CreateBody(w,&bd);
        auto sd=b3DefaultShapeDef();
        if(!joint){auto hull=b3MakeBoxHull(3,.5f,3);b3CreateHullShape(ground,&sd,&hull.base);}
        bd.type=b3_dynamicBody;bd.position={0,joint ? -1.0f : 1.0f,0};bd.linearDamping=0;bd.angularDamping=0;
        auto b=b3CreateBody(w,&bd);auto sphere=b3Sphere{{0,0,0},.5f};b3CreateSphereShape(b,&sd,&sphere);
        if(joint){auto jd=b3DefaultSphericalJointDef();jd.base.bodyIdA=ground;jd.base.bodyIdB=b;jd.base.localFrameB.p={0,1,0};b3CreateSphericalJoint(w,&jd);}
        const char* scene=joint?"spherical-joint":"sphere-ground";
        double total_ms=0;int measured=0;
        for(int step=0;step<240;step++) {
            if(!timing) {
                // Populate caches first; disable, accumulate fresh results, then re-enable.
                if(step==20 || step==100) b3World_EnableWarmStarting(w,false);
                if(step==60 || step==160) b3World_EnableWarmStarting(w,true);
                bool enabled=step<20 || (step>=60 && step<100) || step>=160;
                assert(b3World_IsWarmStartingEnabled(w)==enabled);
#ifdef GPU_API_DUAL
                assert(cpu_b3World_IsWarmStartingEnabled(both_cpu_world(w))==enabled);
#endif
            }
            // Small deterministic loads keep the single constraint exercised.
            b3Body_ApplyForceToCenter(b,{0,float(step%7)*.01f,0},true);
            auto start=std::chrono::steady_clock::now();
            b3World_Step(w,1.f/60,substeps);
            // Public pose read ensures timing includes available completed state.
            auto pos=b3Body_GetPosition(b);assert(std::isfinite(pos.y));
            auto end=std::chrono::steady_clock::now();
            if(step>=60){total_ms+=std::chrono::duration<double,std::milli>(end-start).count();++measured;}
            if(!timing) check_body(b,step,scene);
        }
        printf("{\"scene\":\"%s\",\"substeps\":%d,\"steps\":%d,\"mean_ms\":%.9f,\"timing_only\":%s}\n",scene,substeps,measured,total_ms/measured,timing?"true":"false");
        b3DestroyWorld(w);
    }
}
