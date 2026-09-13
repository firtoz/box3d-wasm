#include "box3d/box3d.h"
#include <cstdio>
extern "C" void b3_world_gpu_wait_with_mirror(b3WorldId) __attribute__((weak));
static void dump(int kind,int phase,b3BodyId b,b3ShapeId s){
 auto m=b3Body_GetMassData(b);auto p=b3Body_GetPosition(b);auto v=b3Body_GetLinearVelocity(b);auto a=b3Body_GetAngularVelocity(b);
 std::printf("%d %d %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g %.9g\n",kind,phase,b3Shape_GetDensity(s),m.mass,m.center.x,m.center.y,m.center.z,m.inertia.cx.x,m.inertia.cy.y,m.inertia.cz.z,m.inertia.cx.y,m.inertia.cx.z,m.inertia.cy.z,p.x,p.y,p.z,v.x,v.y,v.z,a.x,a.y,a.z);
}
int main(){for(int kind=0;kind<5;++kind){
 auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;wd.enableSleep=false;auto w=b3CreateWorld(&wd);
 auto bd=b3DefaultBodyDef();bd.type=kind==3?b3_staticBody:kind==4?b3_kinematicBody:b3_dynamicBody;auto b=b3CreateBody(w,&bd);
 auto sd=b3DefaultShapeDef();sd.density=0;b3ShapeId s={};
 if(kind==0){b3Sphere sphere={{1,2,0},.5f};s=b3CreateSphereShape(b,&sd,&sphere);}
 else if(kind==1){b3Capsule cap={{0,1,0},{1,2,0},.3f};s=b3CreateCapsuleShape(b,&sd,&cap);}
 else{auto h=b3MakeOffsetBoxHull(.3f,.5f,.7f,{1,2,0});s=b3CreateHullShape(b,&sd,&h.base);}
 b3Body_SetAngularVelocity(b,{.1f,.2f,.3f});dump(kind,0,b,s);
 b3Shape_SetDensity(s,2000,false);dump(kind,1,b,s);
 b3Body_ApplyMassFromShapes(b);dump(kind,2,b,s);
 b3Shape_SetDensity(s,0,true);dump(kind,3,b,s);
 b3Shape_SetDensity(s,700,true);dump(kind,4,b,s);
 b3World_Step(w,1.f/60,4);if(b3_world_gpu_wait_with_mirror)b3_world_gpu_wait_with_mirror(w);dump(kind,5,b,s);
 b3DestroyWorld(w);
}}
