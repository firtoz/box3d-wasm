#include "box3d/box3d.h"
#include "benchmarks.h"
#include "human.h"
#include <chrono>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
extern "C" void gpu_b3_world_wait(b3WorldId) __attribute__((weak));
extern "C" uint64_t gpu_b3_world_physics_step(b3WorldId) __attribute__((weak));
extern "C" const char* gpu_b3_world_gpu_fail(b3WorldId) __attribute__((weak));
extern "C" void gpu_b3_world_counts(b3WorldId,int*,int*,int*) __attribute__((weak));
extern "C" void pr04_visit_rain(void(*)(const Human*,int,void*),void*);
struct Profile { uint64_t physics_step,timestamp_step; uint32_t valid,measured_fields; float values[23]; };
extern "C" void gpu_b3_world_native_profile(b3WorldId,Profile*) __attribute__((weak));
extern "C" float gpu_b3_world_last_encode_ms(b3WorldId) __attribute__((weak));
extern "C" float gpu_b3_world_last_fetch_ms(b3WorldId) __attribute__((weak));
using Clock=std::chrono::steady_clock;
static double ms(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
struct Digest {uint64_t hash=1469598103934665603ull;int humans=0,bodies=0;};
static void digest(const Human* h,int group,void* raw){
 auto& d=*(Digest*)raw;++d.humans;
 for(const auto& bone:h->bones){
  assert(b3Body_IsValid(bone.bodyId));auto p=b3Body_GetPosition(bone.bodyId);auto q=b3Body_GetRotation(bone.bodyId);
  auto v=b3Body_GetLinearVelocity(bone.bodyId),w=b3Body_GetAngularVelocity(bone.bodyId);
  float values[]={p.x,p.y,p.z,q.v.x,q.v.y,q.v.z,q.s,v.x,v.y,v.z,w.x,w.y,w.z};
  for(float f:values){assert(std::isfinite(f));uint32_t bits;std::memcpy(&bits,&f,4);d.hash=(d.hash^bits)*1099511628211ull;}++d.bodies;
 }
}
int main(int argc,char** argv){
 assert(argc==2);const bool setters=std::strcmp(argv[1],"viewer-setters")==0;
 assert(setters||std::strcmp(argv[1],"engine-only")==0);setvbuf(stdout,nullptr,_IOLBF,0);
 auto start=Clock::now();auto wd=b3DefaultWorldDef();wd.workerCount=1;GetRainCapacity(&wd.capacity);
 auto world=b3CreateWorld(&wd);assert(b3World_IsValid(world));b3World_SetContactRecycleDistance(world,0.05f);
 CreateRain(world);std::printf("{\"kind\":\"setup\",\"ms\":%.9g,\"variant\":\"%s\",\"gpu\":%s}\n",ms(start,Clock::now()),argv[1],gpu_b3_world_wait?"true":"false");
 for(int frame=0;frame<336;++frame){
  auto a=Clock::now();StepRain(world,frame);auto b=Clock::now();
  if(setters){b3World_EnableSleeping(world,true);b3World_EnableWarmStarting(world,true);b3World_EnableContinuous(world,true);}
  auto c=Clock::now();b3World_Step(world,1.0f/60,4);auto submitted=Clock::now();if(gpu_b3_world_wait)gpu_b3_world_wait(world);auto end=Clock::now();
  uint64_t completed=gpu_b3_world_physics_step?gpu_b3_world_physics_step(world):uint64_t(frame+1);assert(completed==uint64_t(frame+1));
  int bodies=0,shapes=0,joints=0;
  if(gpu_b3_world_counts)gpu_b3_world_counts(world,&bodies,&shapes,&joints);else{const auto n=b3World_GetCounters(world);bodies=n.bodyCount;joints=n.jointCount;}
  if(gpu_b3_world_gpu_fail){const char* fail=gpu_b3_world_gpu_fail(world);assert(!fail||!fail[0]);}
  if(gpu_b3_world_native_profile){
   Profile profile{};gpu_b3_world_native_profile(world,&profile);assert(profile.valid==1);
   auto val=[&](int field){return (profile.measured_fields&(1u<<field))?profile.values[field]:-1.0f;};
   std::printf("{\"kind\":\"profile\",\"step\":%d,\"submit_call_ms\":%.9g,\"completion_call_ms\":%.9g,\"encode_ms\":%.9g,\"fetch_ms\":%.9g,\"timestamp_step\":%llu,\"device_ms\":%.9g,\"broadphase_ms\":%.9g,\"collide_ms\":%.9g,\"solver_ms\":%.9g}\n",frame+1,ms(c,submitted),ms(submitted,end),gpu_b3_world_last_encode_ms(world),gpu_b3_world_last_fetch_ms(world),(unsigned long long)profile.timestamp_step,val(0),val(1),val(2),val(3));
  }
  std::printf("{\"kind\":\"step\",\"step\":%d,\"completed\":%llu,\"create_ms\":%.9g,\"setters_ms\":%.9g,\"physics_ms\":%.9g,\"bodies\":%d,\"joints\":%d}\n",frame+1,(unsigned long long)completed,ms(a,b),ms(b,c),ms(c,end),bodies,joints);
 }
 Digest d;pr04_visit_rain(digest,&d);assert(d.humans==300&&d.bodies==4200);
 std::printf("{\"kind\":\"endpoint\",\"humans\":%d,\"dynamic_bodies\":%d,\"pose_velocity_digest\":\"%016llx\"}\n",d.humans,d.bodies,(unsigned long long)d.hash);
 DestroyRain();b3DestroyWorld(world);std::puts("PASS profiled original Rain336 completed");
}
