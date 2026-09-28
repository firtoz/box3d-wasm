// Native tree events for the independent ordering regression fixture.
// Link wrappers observe metadata; they never alter tree or simulation state.
#include "physics_world.h"
#include "box3d/collision.h"
#include "contact.h"
#include "shape.h"
#include <stdio.h>
static int frame;
static b3DynamicTree* dynamic;
static bool visit(int proxy,uint64_t data,void* ctx){
 (void)ctx;fprintf(stderr,"leaf %d %d %llu\n",frame,proxy,(unsigned long long)data+1);return true;
}
static bool pair_visit(int proxy,uint64_t data,void* ctx){
 int type=*(int*)ctx;
 fprintf(stderr,"pair-leaf %d %d %llu\n",frame,B3_PROXY_KEY(proxy,type),(unsigned long long)data+1);
 return true;
}
static void bounds(const char* name,b3DynamicTree* tree,int proxy,b3AABB a){
 fprintf(stderr,"tree-%s %d %d %llu %.9g %.9g %.9g %.9g %.9g %.9g\n",name,frame,proxy,(unsigned long long)b3DynamicTree_GetUserData(tree,proxy)+1,a.lowerBound.x,a.lowerBound.y,a.lowerBound.z,a.upperBound.x,a.upperBound.y,a.upperBound.z);
}
void __real_b3World_Step(b3WorldId,float,int);
void __wrap_b3World_Step(b3WorldId id,float dt,int n){
 ++frame;b3World* w=b3GetWorldFromId(id);dynamic=w->broadPhase.trees+b3_dynamicBody;
 if(frame==1)for(int i=0;i<dynamic->nodeCapacity;i++)if((dynamic->nodes[i].flags&b3_leafNode)!=0)bounds("initial",dynamic,i,dynamic->nodes[i].aabb);
 __real_b3World_Step(id,dt,n);
}
void __real_b3UpdateBroadPhasePairs(b3World*);
void __wrap_b3UpdateBroadPhasePairs(b3World* w){
 fprintf(stderr,"query-begin %d\n",frame);
 b3AABB all={{-1000,-1000,-1000},{1000,1000,1000}};
 b3DynamicTree_Query(dynamic,all,UINT64_MAX,false,visit,NULL);
 for(int type=0;type<b3_bodyTypeCount;type++)
  b3DynamicTree_Query(w->broadPhase.trees+type,all,UINT64_MAX,false,pair_visit,&type);
 for(int i=0;i<w->broadPhase.moveArray.count;i++)fprintf(stderr,"moved %d %d\n",frame,w->broadPhase.moveArray.data[i]);
 __real_b3UpdateBroadPhasePairs(w);
}
void __real_b3CreateContact(b3World*,b3Shape*,b3Shape*,int);
void __wrap_b3CreateContact(b3World* w,b3Shape* a,b3Shape* b,int child){
 int before=b3GetIdCount(&w->contactIdPool);
 __real_b3CreateContact(w,a,b,child);
 if(b3GetIdCount(&w->contactIdPool)>before)
  fprintf(stderr,"pair-created %d %d %d\n",frame,a->id+1,b->id+1);
}
void __real_b3DynamicTree_EnlargeProxy(b3DynamicTree*,int,b3AABB);
void __wrap_b3DynamicTree_EnlargeProxy(b3DynamicTree* t,int id,b3AABB a){
 if(t==dynamic)bounds("enlarge",t,id,a);
 __real_b3DynamicTree_EnlargeProxy(t,id,a);
}
void __real_b3DynamicTree_MoveProxy(b3DynamicTree*,int,b3AABB);
void __wrap_b3DynamicTree_MoveProxy(b3DynamicTree* t,int id,b3AABB a){
 if(t==dynamic)bounds("move",t,id,a);
 __real_b3DynamicTree_MoveProxy(t,id,a);
}
int __real_b3DynamicTree_Rebuild(b3DynamicTree*,bool);
int __wrap_b3DynamicTree_Rebuild(b3DynamicTree* t,bool full){
 if(t==dynamic)fprintf(stderr,"tree-rebuild %d %d\n",frame,full);
 return __real_b3DynamicTree_Rebuild(t,full);
}
