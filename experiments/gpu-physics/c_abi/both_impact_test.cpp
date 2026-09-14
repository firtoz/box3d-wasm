// Cold-start replay from an airborne, contact-free state captured by both_drag.
// This is a diagnostic, not a replacement for the retained-history drag gate.
#include "both_view.h"
extern "C" {
#include "both_ids.h"
b3WorldTransform cpu_b3Body_GetTransform(b3BodyId);
b3Vec3 cpu_b3Body_GetLinearVelocity(b3BodyId);
b3Vec3 cpu_b3Body_GetAngularVelocity(b3BodyId);
int cpu_b3Body_GetContactData(b3BodyId, b3ContactData*, int);
void gpu_b3_world_wait(b3WorldId);
void SetSelectedBody(b3BodyId) {}
void SetComparisonSelectedBody(b3BodyId) {}
}
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
int main(int argc, char** argv) {
  assert(argc == 3);
  FILE* input = std::fopen(argv[1], "r"); assert(input);
  float p[3], q[4], v[3], w[3];
  assert(std::fscanf(input, "%f %f %f %f %f %f %f %f %f %f %f %f %f",
    p,p+1,p+2,q,q+1,q+2,q+3,v,v+1,v+2,w,w+1,w+2) == 13);
  std::fclose(input);
  int steps = std::atoi(argv[2]); assert(steps > 0 && steps <= 300);
  auto wd = b3DefaultWorldDef(); auto world = b3CreateWorld(&wd);
  auto sd = b3DefaultShapeDef(); auto bd = b3DefaultBodyDef();
  bd.position = {0,-.5,0}; auto floor = b3CreateBody(world,&bd);
  auto ground = b3MakeBoxHull(20,.5,20); b3CreateHullShape(floor,&sd,&ground.base);
  bd.type = b3_dynamicBody; bd.position = {p[0],p[1],p[2]};
  bd.rotation = {{q[0],q[1],q[2]},q[3]};
  auto body = b3CreateBody(world,&bd); auto box = b3MakeBoxHull(.5,.5,.5);
  b3CreateHullShape(body,&sd,&box.base);
  b3Body_SetLinearVelocity(body,{v[0],v[1],v[2]});
  b3Body_SetAngularVelocity(body,{w[0],w[1],w[2]});
  for(int frame=0;frame<=steps;++frame) {
    if(frame) { b3World_Step(world,1.f/60,4); gpu_b3_world_wait(world); }
    for(int engine=0;engine<2;++engine) {
      auto id = engine ? body : both_cpu_body(body);
      auto t = engine ? b3Body_GetTransform(id) : cpu_b3Body_GetTransform(id);
      auto lv = engine ? b3Body_GetLinearVelocity(id) : cpu_b3Body_GetLinearVelocity(id);
      auto av = engine ? b3Body_GetAngularVelocity(id) : cpu_b3Body_GetAngularVelocity(id);
      if (frame == 0) {
        const float actual[] = {float(t.p.x),float(t.p.y),float(t.p.z),t.q.v.x,t.q.v.y,t.q.v.z,t.q.s,lv.x,lv.y,lv.z,av.x,av.y,av.z};
        const float expected[] = {p[0],p[1],p[2],q[0],q[1],q[2],q[3],v[0],v[1],v[2],w[0],w[1],w[2]};
        for (int i=0;i<13;++i) assert(std::isfinite(actual[i]) && std::abs(actual[i]-expected[i]) < 2e-6f);
      }
      std::printf("F %d %d p %.9g %.9g %.9g q %.9g %.9g %.9g %.9g v %.9g %.9g %.9g w %.9g %.9g %.9g\n",
        frame,engine,t.p.x,t.p.y,t.p.z,t.q.v.x,t.q.v.y,t.q.v.z,t.q.s,lv.x,lv.y,lv.z,av.x,av.y,av.z);
      b3ContactData contacts[4]; int n = engine ? b3Body_GetContactData(id,contacts,4) : cpu_b3Body_GetContactData(id,contacts,4);
      for(int c=0;c<n;++c) for(int m=0;m<contacts[c].manifoldCount;++m) {
        auto& manifold=contacts[c].manifolds[m];
        std::printf("M %d %d count %d normal %.9g %.9g %.9g\n",frame,engine,manifold.pointCount,manifold.normal.x,manifold.normal.y,manifold.normal.z);
        for(int i=0;i<manifold.pointCount;++i) {
          auto& pt=manifold.points[i];
          std::printf("P %d %d id %u a %.9g %.9g %.9g b %.9g %.9g %.9g sep %.9g imp %.9g\n",frame,engine,pt.featureId,pt.anchorA.x,pt.anchorA.y,pt.anchorA.z,pt.anchorB.x,pt.anchorB.y,pt.anchorB.z,pt.separation,pt.normalImpulse);
        }
      }
    }
  }
  b3DestroyWorld(world);
}
