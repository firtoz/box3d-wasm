#include "box3d/box3d.h"
#include <cstdio>
#ifdef GPU_REFERENCE
extern "C" void gpu_b3_world_counts(b3WorldId,int*,int*,int*);
extern "C" void gpu_b3_destroy_joint(b3JointId,bool);
// These two wrappers live in samples_api.c, not the core shim archive.
#define b3DestroyJoint gpu_b3_destroy_joint
#endif
static bool check(b3WorldId world,int phase,int bodies,int shapes,int joints) {
 int b,s,j;
#ifdef GPU_REFERENCE
 gpu_b3_world_counts(world,&b,&s,&j);
#else
 auto c=b3World_GetCounters(world);b=c.bodyCount;s=c.shapeCount;j=c.jointCount;
#endif
 printf("%d %d %d %d\n",phase,b,s,j);
 return b==bodies && s==shapes && j==joints;
}
int main(){
 auto wd=b3DefaultWorldDef();auto world=b3CreateWorld(&wd);
 auto bd=b3DefaultBodyDef();auto ground=b3CreateBody(world,&bd);
 b3CompoundSphereDef children[2]={};
 for(int i=0;i<2;++i){children[i].sphere={{float(4*i),0,0},0.5f};children[i].material=b3DefaultSurfaceMaterial();}
 b3CompoundDef cd={};cd.spheres=children;cd.sphereCount=2;
 auto compound=b3CreateCompound(&cd);auto sd=b3DefaultShapeDef();
 auto parent=b3CreateBakedCompoundShape(ground,&sd,compound);
 bd.type=b3_dynamicBody;auto body=b3CreateBody(world,&bd);
 b3Sphere sphere={b3Vec3_zero,0.5f};auto shape=b3CreateSphereShape(body,&sd,&sphere);
 auto jd=b3DefaultRevoluteJointDef();jd.base.bodyIdA=ground;jd.base.bodyIdB=body;
 auto first=b3CreateRevoluteJoint(world,&jd),second=b3CreateRevoluteJoint(world,&jd);
 bool ok=check(world,0,2,2,2);
 b3DestroyJoint(first,false);ok &=check(world,1,2,2,1);
 b3DestroyJoint(second,false);b3DestroyShape(shape,true);ok &=check(world,2,2,1,0);
 b3DestroyShape(parent,false);ok &=check(world,3,2,0,0);
 b3DestroyWorld(world);b3DestroyCompound(compound);return ok?0:32;
}
