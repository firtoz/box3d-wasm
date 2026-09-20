#include "box3d/box3d.h"
extern "C" {
#include "both_ids.h"
b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId);
b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId);
b3WorldTransform cpu_b3Body_GetTransform(b3BodyId);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
static int config, substeps, phase, checks;
static void near(float g, float c, const char* label) {
  if (!(std::isfinite(g) && std::isfinite(c) &&
        fabsf(g-c) <= 1e-5f * fmaxf(1.f, fmaxf(fabsf(g),fabsf(c))))) {
    fprintf(stderr,"config=%d substeps=%d phase=%d %s GPU=%.9g CPU=%.9g\n",config,substeps,phase,label,g,c);
    abort();
  }
  ++checks;
}
static void inspect(b3BodyId b) {
  auto c=both_cpu_body(b);
  auto gv=b3Body_GetLinearVelocity(b), cv=cpu_b3Body_GetLinearVelocity(c);
  auto gw=b3Body_GetAngularVelocity(b), cw=cpu_b3Body_GetAngularVelocity(c);
  for (int i=0;i<3;++i) {near((&gv.x)[i],(&cv.x)[i],"velocity");near((&gw.x)[i],(&cw.x)[i],"omega");}
  auto gt=b3Body_GetTransform(b), ct=cpu_b3Body_GetTransform(c);
  near(gt.p.x,ct.p.x,"position.x"); near(gt.p.y,ct.p.y,"position.y"); near(gt.p.z,ct.p.z,"position.z");
  near(gt.q.v.x,ct.q.v.x,"rotation.x");near(gt.q.v.y,ct.q.v.y,"rotation.y");near(gt.q.v.z,ct.q.v.z,"rotation.z");near(gt.q.s,ct.q.s,"rotation.s");
}
static b3BodyId body(b3WorldId w, bool asleep=false) {
  auto d=b3DefaultBodyDef(); d.type=b3_dynamicBody; d.isAwake=!asleep;
  d.linearDamping=config&1 ? .7f : 0; d.angularDamping=config&1 ? .4f : 0;
  if(config>=2) d.rotation=b3MakeQuatFromAxisAngle({0,1,0},.4f);
  auto b=b3CreateBody(w,&d); auto sd=b3DefaultShapeDef();sd.density=1;
  if(config>=2) {auto hull=b3MakeBoxHull(.5f,.7f,.9f);b3CreateHullShape(b,&sd,&hull.base);}
  else {b3Sphere sphere={{0,0,0},.5f};b3CreateSphereShape(b,&sd,&sphere);}
  return b;
}
static void load(b3BodyId b,bool wake) {
  b3Body_ApplyForceToCenter(b,{.2f,.4f,-.3f},wake);
  b3Body_ApplyForceToCenter(b,{-.1f,.2f,.1f},wake);
  b3Body_ApplyTorque(b,{.03f,.02f,-.01f},wake);
}
int main() {
  for(config=0;config<4;++config) for(int count : {1,2,4,8}) {
    substeps=count;
    auto wd=b3DefaultWorldDef();wd.gravity=config&1 ? b3Vec3{0,-1,0} : b3Vec3{0,0,0};auto w=b3CreateWorld(&wd);auto b=body(w);
    phase=0; load(b,true);inspect(b); // forces must not change the current velocity
    b3World_Step(w,0,substeps);inspect(b); // zero dt must not consume the load
    phase=1;b3World_Step(w,1.f/60,substeps);inspect(b);
    // Unloaded steps exercise load clearing and resident/full-replay eligibility.
    for(phase=2;phase<6;++phase) {b3World_Step(w,1.f/120,substeps);inspect(b);}
    phase=6;load(b,true);b3Body_SetLinearDamping(b,.6f); // rebuild with a pending load
    b3World_Step(w,1.f/30,substeps);inspect(b);
    phase=7;auto p=b3Body_GetPosition(b);
    b3Body_ApplyForce(b,{.1f,.2f,.3f},{p.x+.2f,p.y+.1f,p.z-.1f},true);
    b3World_Step(w,1.f/60,substeps);inspect(b);
    phase=8;b3Body_SetAwake(b,false);load(b,false);b3World_Step(w,1.f/60,substeps);inspect(b);
    phase=9;load(b,true);b3World_Step(w,1.f/60,substeps);inspect(b);
    phase=10;load(b,true);b3DestroyBody(b);b=body(w); // pending load cannot reach a reused slot
    b3World_Step(w,1.f/60,substeps);inspect(b);
    phase=11;load(b,true);b3World_Step(w,1.f/60,substeps);inspect(b);
    b3DestroyBody(b);b=body(w); // a GPU-resident old load cannot reach a reused slot
    auto other=body(w);
    b3Body_SetTransform(other,{10,0,0},b3Quat_identity);
    phase=12;load(other,true);b3World_Step(w,1.f/60,substeps);inspect(b);inspect(other);
    // Keep the second slot live through a hole and reuse of the first slot.
    phase=13;b3DestroyBody(b);load(other,true);b3World_Step(w,1.f/60,substeps);inspect(other);
    phase=14;b=body(w);load(b,true);b3World_Step(w,1.f/60,substeps);inspect(b);inspect(other);
    b3DestroyWorld(w);
  }
  printf("substep forces: %d independent CPU/GPU scalar comparisons passed\n",checks);
}
