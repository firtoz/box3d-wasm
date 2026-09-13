#include "box3d/box3d.h"
#include <cstdio>
extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));
int main() {
 for(int kind=0;kind<4;++kind) {
  auto wd=b3DefaultWorldDef();wd.enableSleep=false;
  auto w=b3CreateWorld(&wd);
  auto bd=b3DefaultBodyDef();bd.type=b3_dynamicBody;bd.position={0,5,0};
  auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();auto h=b3MakeBoxHull(.3f,.5f,.7f);
  b3CreateHullShape(b,&sd,&h.base);
  auto md=b3Body_GetMassData(b);md.mass=0;if(kind==2)md.inertia={};b3Body_SetMassData(b,md);
  b3Body_SetLinearVelocity(b,{.2f,0,0});b3Body_SetAngularVelocity(b,{0,0,.1f});
  if(kind==0)b3Body_ApplyAngularImpulse(b,{0,0,.01f},true);
  if(kind==1)b3Body_ApplyLinearImpulse(b,{0,.01f,0},{1,5,0},true);
  if(kind==3) {
   auto groundDef=b3DefaultBodyDef();groundDef.position={0,5,0};auto ground=b3CreateBody(w,&groundDef);
   auto jd=b3DefaultRevoluteJointDef();jd.base.bodyIdA=ground;jd.base.bodyIdB=b;
   jd.enableMotor=true;jd.motorSpeed=.7f;jd.maxMotorTorque=.1f;b3CreateRevoluteJoint(w,&jd);
   b3Body_SetLinearVelocity(b,b3Vec3_zero);b3Body_SetAngularVelocity(b,b3Vec3_zero);
  }
  for(int step=0;step<=30;++step) {
   if(step){if(step==1)b3Body_ApplyTorque(b,{0,0,.1f},true);b3World_Step(w,1.f/60,4);}
   if(b3_world_gpu_wait_with_mirror)b3_world_gpu_wait_with_mirror(w);
   auto p=b3Body_GetPosition(b);auto q=b3Body_GetRotation(b);auto v=b3Body_GetLinearVelocity(b);auto a=b3Body_GetAngularVelocity(b);
   std::printf("%d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",kind,step,p.x,p.y,p.z,q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,a.x,a.y,a.z);
  }
  b3DestroyWorld(w);
 }
}
