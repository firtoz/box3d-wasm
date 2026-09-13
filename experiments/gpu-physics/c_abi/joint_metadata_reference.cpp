#include "box3d/box3d.h"
#include <cstdio>
#include <vector>
static bool same(b3BodyId a,b3BodyId b){return a.index1==b.index1&&a.world0==b.world0&&a.generation==b.generation;}
static bool same(b3WorldId a,b3WorldId b){return a.index1==b.index1&&a.generation==b.generation;}
int main(){auto wd=b3DefaultWorldDef();auto w=b3CreateWorld(&wd);auto bd=b3DefaultBodyDef();auto a=b3CreateBody(w,&bd);
 bd.type=b3_dynamicBody;auto hole=b3CreateBody(w,&bd);b3DestroyBody(hole);auto b=b3CreateBody(w,&bd);
 std::printf("generation-reused %d\n",b.generation!=hole.generation);
 auto sd=b3DefaultShapeDef();b3Sphere sphere={{0,0,0},.5f};auto s=b3CreateSphereShape(b,&sd,&sphere);
 std::printf("worlds %d %d\n",same(b3Body_GetWorld(b),w),same(b3Shape_GetWorld(s),w));
 std::vector<b3JointId> joints;
#define ADD(Name) {auto d=b3Default##Name##JointDef();d.base.bodyIdA=a;d.base.bodyIdB=b;joints.push_back(b3Create##Name##Joint(w,&d));}
 ADD(Parallel);ADD(Distance);ADD(Filter);ADD(Motor);ADD(Prismatic);ADD(Revolute);ADD(Spherical);ADD(Weld);ADD(Wheel);
 for(int i=0;i<9;++i){auto j=joints[i];std::printf("joint %d %d %d %d %d\n",i,int(b3Joint_GetType(j)),same(b3Joint_GetBodyA(j),a),same(b3Joint_GetBodyB(j),b),same(b3Joint_GetWorld(j),w));}
 b3DestroyBody(b);auto reuse=b3CreateBody(w,&bd);(void)reuse;
 for(int i=0;i<9;++i)std::printf("destroyed %d %d\n",i,b3Joint_IsValid(joints[i]));
 b3DestroyWorld(w);
}
