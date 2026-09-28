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
 // Filters retain the complete base definition even though they apply no forces.
 auto checkFilter = [&](const b3FilterJointDef& d) {
  auto j=b3CreateFilterJoint(w,&d);
  auto fa=b3Joint_GetLocalFrameA(j),fb=b3Joint_GetLocalFrameB(j);
  auto equal=[](b3Transform x,b3Transform y){return x.p.x==y.p.x&&x.p.y==y.p.y&&x.p.z==y.p.z&&x.q.v.x==y.q.v.x&&x.q.v.y==y.q.v.y&&x.q.v.z==y.q.v.z&&x.q.s==y.q.s;};
  float hertz,damping;b3Joint_GetConstraintTuning(j,&hertz,&damping);
  bool ok=equal(fa,d.base.localFrameA)&&equal(fb,d.base.localFrameB)&&hertz==d.base.constraintHertz&&damping==d.base.constraintDampingRatio;
  return ok; // The body destruction below cleans up these attached joints.
 };
 auto filter=b3DefaultFilterJointDef();filter.base.bodyIdA=a;filter.base.bodyIdB=b;
 std::printf("filter-default %d\n",checkFilter(filter));
 filter.base.localFrameA.p={1,2,3};filter.base.localFrameB.p={-4,5,-6};
 filter.base.localFrameA.q={{0,1,0},0};filter.base.localFrameB.q={{1,0,0},0};
 filter.base.constraintHertz=37;filter.base.constraintDampingRatio=.75f;
 std::printf("filter-custom %d\n",checkFilter(filter));
 b3DestroyBody(b);auto reuse=b3CreateBody(w,&bd);(void)reuse;
 for(int i=0;i<9;++i)std::printf("destroyed %d %d\n",i,b3Joint_IsValid(joints[i]));
 b3DestroyWorld(w);
}
