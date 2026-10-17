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
{b3Vec3 points[]={{4.00000000f,-0.113281250f,49.0000000f},{4.00000000f,-0.214843750f,48.0000000f},{3.00000000f,-0.0898437500f,49.0000000f}};trace(120,4711,points,{-23.5000000f,2.00000000f,23.9999733f},0.100000001f,{80000.0000f,-80000.0000f,8.00000000f});}
{b3Vec3 points[]={{3.00000000f,0.675781250f,12.0000000f},{3.00000000f,0.675781250f,13.0000000f},{4.00000000f,0.839843750f,12.0000000f}};trace(391,1182,points,{-22.3000011f,2.00000000f,-12.8000107f},0.100000001f,{80000.0000f,-80000.0000f,8.00000000f});}
{b3Vec3 points[]={{4.00000000f,0.839843750f,13.0000000f},{4.00000000f,0.839843750f,12.0000000f},{3.00000000f,0.675781250f,13.0000000f}};trace(391,1183,points,{-22.3000011f,2.00000000f,-12.8000107f},0.100000001f,{80000.0000f,-80000.0000f,8.00000000f});}
}
