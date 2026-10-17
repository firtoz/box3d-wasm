import {resolve} from "node:path";
const dir=resolve("experiments/gpu-physics/benchmarks/production-readiness/pr11-queries-2026-10-17");
const label=process.argv[2]??"triangle-trace";
const data=await Bun.file(resolve(dir,`${label}-inputs.json`)).json();
const vec=(a:number[])=>"{"+a.map(x=>x.toPrecision(9)+"f").join(",")+"}";
const source=`#include "box3d/collision.h"
#include <cstdio>
static void trace(int index,int id,b3Vec3* local,b3Vec3 worldPoint,float radius,b3Vec3 delta) {
 for(int frame=0;frame<2;++frame){
  b3Vec3 triangle[3];for(int i=0;i<3;++i)triangle[i]=frame?b3Sub(local[i],local[0]):b3Add(local[i],{-25,0,-25});
  b3Vec3 point=frame?b3Add(worldPoint,{25,0,25}):worldPoint;
  b3Transform transform=b3Transform_identity;if(frame)transform.p=b3Neg(local[0]);
  b3ShapeCastPairInput input={};input.proxyA={triangle,3,0};input.proxyB={&point,1,radius};input.translationB=delta;input.transform=transform;input.maxFraction=1;
  b3DistanceInput distance={};distance.proxyA=input.proxyA;distance.proxyB=input.proxyB;distance.transform=transform;distance.useRadii=false;b3SimplexCache cache={};auto d=b3ShapeDistance(&distance,&cache,nullptr,0);
  auto c=b3ShapeCast(&input);auto p=c.point;if(frame)p=b3Add(b3Add(p,local[0]),{-25,0,-25});
  printf("{\\\"case\\\":%d,\\\"triangle\\\":%d,\\\"local_frame\\\":%d,\\\"initial_distance\\\":%.9g,\\\"initial_normal\\\":[%.9g,%.9g,%.9g],\\\"hit\\\":%d,\\\"fraction\\\":%.9g,\\\"point\\\":[%.9g,%.9g,%.9g],\\\"normal\\\":[%.9g,%.9g,%.9g]}\\n",index,id,frame,d.distance,d.normal.x,d.normal.y,d.normal.z,c.hit,c.fraction,p.x,p.y,p.z,c.normal.x,c.normal.y,c.normal.z);
 }
}
int main(){
${data.flatMap((row:any)=>row.triangles.map((t:any)=>`{b3Vec3 points[]={${t.points.map(vec).join(",")}};trace(${row.input.index},${t.id},points,${vec(row.input.origin)},${row.input.radius.toPrecision(9)}f,${vec(row.input.translation)});}`)).join("\n")}
}
`;
await Bun.write(resolve(dir,`${label}.cpp`),source);
