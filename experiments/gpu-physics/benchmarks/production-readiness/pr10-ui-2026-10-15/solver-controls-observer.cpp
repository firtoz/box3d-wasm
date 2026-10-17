#include "box3d/id.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
static void record(const char* api,b3WorldId id,float value,int substeps=-1) {
 const char* path=std::getenv("GPU_PR10_API_TRACE");if(!path)return;
 if(FILE* f=std::fopen(path,"a")) {
  std::fprintf(f,"{\"api\":\"%s\",\"world\":[%u,%u],\"value\":%.9g,\"substeps\":%d}\n",api,(unsigned)id.index1,(unsigned)id.generation,value,substeps);std::fclose(f);
 }
}
#define STEP(name) extern "C" void __real_##name(b3WorldId,float,int) __attribute__((weak)); extern "C" void __wrap_##name(b3WorldId id,float dt,int n) { assert(__real_##name);record(#name,id,dt,n);__real_##name(id,dt,n); }
#define RECYCLE(name) extern "C" void __real_##name(b3WorldId,float) __attribute__((weak)); extern "C" void __wrap_##name(b3WorldId id,float v) { assert(__real_##name);record(#name,id,v);__real_##name(id,v); }
STEP(b3World_Step)
STEP(cpu_b3World_Step)
STEP(gpu_b3_world_step)
RECYCLE(b3World_SetContactRecycleDistance)
RECYCLE(cpu_b3World_SetContactRecycleDistance)
RECYCLE(gpu_b3_world_set_contact_recycle_distance)
