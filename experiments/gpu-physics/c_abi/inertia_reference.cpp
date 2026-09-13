#include <initializer_list>
#include "box3d/box3d.h"
#include <cstdio>
static void dump(int kind,int phase,b3BodyId b) {
 auto local=b3Body_GetLocalRotationalInertia(b);
 auto world=b3Body_GetWorldInverseRotationalInertia(b);
 std::printf("%d %d",kind,phase);
 for(auto m:{local,world}) {
  for(auto v:{m.cx,m.cy,m.cz})std::printf(" %.9g %.9g %.9g",v.x,v.y,v.z);
 }
 std::puts("");
}
int main(){
 auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;auto w=b3CreateWorld(&wd);
 for(int kind=0;kind<6;kind++){
  auto bd=b3DefaultBodyDef();bd.type=kind==0?b3_staticBody:kind==1?b3_kinematicBody:b3_dynamicBody;
  bd.position={(float)kind*10,0,0};bd.rotation=b3MakeQuatFromAxisAngle({0,0,1},.6f);
  bd.motionLocks.angularX=kind>=3;bd.motionLocks.angularY=kind==4;bd.motionLocks.angularZ=kind==4;
  auto b=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();auto hull=b3MakeBoxHull(.3f,.5f,.7f);b3CreateHullShape(b,&sd,&hull.base);
  if(kind==5){auto md=b3Body_GetMassData(b);md.mass=0;md.inertia={};b3Body_SetMassData(b,md);}
  dump(kind,0,b);
  b3Body_SetTransform(b,{(float)kind*10,0,0},b3MakeQuatFromAxisAngle({0,1,0},1.1f));dump(kind,1,b);
  b3Body_SetAngularVelocity(b,{.3f,.2f,.1f});b3World_Step(w,1.f/60,4);dump(kind,2,b);
  b3MotionLocks locks={};b3Body_SetMotionLocks(b,locks);dump(kind,3,b);
 }
 b3DestroyWorld(w);
}
