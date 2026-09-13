#include "box3d/box3d.h"
#include <cstdio>
#include <vector>
#ifdef GPU_REFERENCE
#include "../native-samples/contact_metrics.h"
#endif
// Native oracle: compound child contacts are distinct; sensor overlaps are not contacts.
int main(){
 for(int compoundCase=0;compoundCase<2;++compoundCase)for(int sensor=0;sensor<2;++sensor)for(int nearMiss=0;nearMiss<2;++nearMiss){
  auto wd=b3DefaultWorldDef();wd.gravity=b3Vec3_zero;auto w=b3CreateWorld(&wd);
  auto bd=b3DefaultBodyDef();auto ground=b3CreateBody(w,&bd);auto sd=b3DefaultShapeDef();
  b3CompoundData* compound=nullptr;
  if(compoundCase){
   b3CompoundSphereDef children[2]={};
   for(int i=0;i<2;++i){children[i].sphere={{i?0.25f:-0.25f,0,0},1};children[i].material=b3DefaultSurfaceMaterial();}
   b3CompoundDef cd={};cd.spheres=children;cd.sphereCount=2;compound=b3CreateCompound(&cd);
   b3CreateBakedCompoundShape(ground,&sd,compound);
  }else{b3Sphere sphere={b3Vec3_zero,1};b3CreateSphereShape(ground,&sd,&sphere);}
  bd.type=b3_dynamicBody;bd.position={nearMiss?1.8f:0.0f,1.5f,0};auto body=b3CreateBody(w,&bd);
  sd.isSensor=sensor;sd.density=1;b3Sphere sphere={b3Vec3_zero,1};auto shape=b3CreateSphereShape(body,&sd,&sphere);
  auto print=[&](const char* phase){
    int capacity=b3Body_GetContactCapacity(body);
    std::vector<b3ContactData> contacts(capacity);
    int touching=b3Body_GetContactData(body,contacts.data(),capacity);
#ifdef GPU_REFERENCE
    GpuContactMetrics metric={};gpu_b3_world_contact_metrics(w,&metric);
    printf("compound=%d sensor=%d near_miss=%d phase=%s contacts=-1 body_capacity=%d touching=%d roots=%u non_sensor_roots=%u known=%u current=%u step=%llu\n",compoundCase,sensor,nearMiss,phase,capacity,touching,metric.allocated_roots,metric.non_sensor_roots,metric.known,metric.current,(unsigned long long)metric.snapshot_step);
#else
    auto c=b3World_GetCounters(w);
    printf("compound=%d sensor=%d near_miss=%d phase=%s contacts=%d body_capacity=%d touching=%d\n",compoundCase,sensor,nearMiss,phase,c.contactCount,capacity,touching);
#endif
  };
  print("created");b3World_Step(w,1.0f/60.0f,4);print("stepped");
  b3Body_SetTransform(body,{0,100,0},b3Quat_identity);print("teleported");
  b3World_Step(w,1.0f/60.0f,4);print("separated");
  b3Body_SetTransform(body,{nearMiss?1.8f:0.0f,1.5f,0},b3Quat_identity);b3World_Step(w,1.0f/60.0f,4);print("returned");
  b3DestroyShape(shape,true);print("destroyed_shape");b3DestroyWorld(w);if(compound)b3DestroyCompound(compound);
 }
}
