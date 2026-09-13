#include "box3d/box3d.h"
#include <cstdio>
int main() {
 const float thresholds[]={0,1,3};
 const float speeds[]={0.5f,1,2,4};
 for(int mode=0;mode<2;++mode) for(int ti=0;ti<3;++ti) for(int si=0;si<4;++si) {
  auto wd=b3DefaultWorldDef();wd.gravity={0,0,0};wd.enableSleep=false;
  if(mode==0)wd.restitutionThreshold=thresholds[ti];
  auto w=b3CreateWorld(&wd);auto bd=b3DefaultBodyDef();auto a=b3CreateBody(w,&bd);
  bd.type=b3_dynamicBody;bd.position={0,mode?10.0f:1.0f,0};auto b=b3CreateBody(w,&bd);
  auto sd=b3DefaultShapeDef();sd.baseMaterial.restitution=0.8f;sd.baseMaterial.friction=0;
  b3Sphere sphere={{0,0,0},0.5f};b3CreateSphereShape(a,&sd,&sphere);b3CreateSphereShape(b,&sd,&sphere);
  if(mode) {
   b3World_Step(w,1.f/60,4);
   b3World_SetRestitutionThreshold(w,ti==0?-2.0f:thresholds[ti]);
   b3Body_SetTransform(b,{0,1,0},b3Quat_identity);
  }
  b3Body_SetLinearVelocity(b,{0,-speeds[si],0});
  b3World_Step(w,1.f/60,4);
  auto v=b3Body_GetLinearVelocity(b);auto p=b3Body_GetPosition(b);
  std::printf("%d %d %d %.9g %.9g %.9g\n",mode,ti,si,b3World_GetRestitutionThreshold(w),v.y,p.y);
  b3DestroyWorld(w);
 }
}
