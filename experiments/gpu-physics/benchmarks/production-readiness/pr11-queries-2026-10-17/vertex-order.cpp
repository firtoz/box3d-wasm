#include "box3d/collision.h"
#include <cstdio>
static void trace(int index,int id,b3Vec3* local,b3Vec3 worldPoint,float radius,b3Vec3 delta) {
 for(int frame=0;frame<2;++frame){
  b3Vec3 triangle[3];for(int i=0;i<3;++i)triangle[i]=frame?b3Sub(local[i],local[0]):b3Add(local[i],{-25,0,-25});
  b3Vec3 point=frame?b3Add(worldPoint,{25,0,25}):worldPoint;
  b3Transform transform=b3Transform_identity;if(frame)transform.p=b3Neg(local[0]);
  b3ShapeCastPairInput input={};input.proxyA={triangle,3,0};input.proxyB={&point,1,radius};input.translationB=delta;input.transform=transform;input.maxFraction=1;
  b3DistanceInput distance={};distance.proxyA=input.proxyA;distance.proxyB=input.proxyB;distance.transform=transform;distance.useRadii=false;b3SimplexCache cache={};auto d=b3ShapeDistance(&distance,&cache,nullptr,0);
  auto c=b3ShapeCast(&input);auto p=c.point;if(frame)p=b3Add(b3Add(p,local[0]),{-25,0,-25});
  printf("{\"case\":%d,\"triangle\":%d,\"local_frame\":%d,\"initial_distance\":%.9g,\"initial_normal\":[%.9g,%.9g,%.9g],\"hit\":%d,\"fraction\":%.9g,\"point\":[%.9g,%.9g,%.9g],\"normal\":[%.9g,%.9g,%.9g]}\n",index,id,frame,d.distance,d.normal.x,d.normal.y,d.normal.z,c.hit,c.fraction,p.x,p.y,p.z,c.normal.x,c.normal.y,c.normal.z);
 }
}
int main(){
{b3Vec3 points[]={{33.0000000f,0.222656250f,23.0000000f},{34.0000000f,0.191406250f,23.0000000f},{34.0000000f,0.277343750f,22.0000000f}};trace(9372,2223,points,{7.29998779f,2.00000000f,-2.00001383f},0.100000001f,{80000.0000f,-80000.0000f,8.00000000f});}
{b3Vec3 points[]={{33.0000000f,0.761718750f,8.00000000f},{34.0000000f,0.644531250f,8.00000000f},{34.0000000f,0.589843750f,7.00000000f}};trace(9453,753,points,{7.69998789f,2.00000000f,-18.0000057f},0.100000001f,{80000.0000f,-80000.0000f,8.00000000f});}
}
