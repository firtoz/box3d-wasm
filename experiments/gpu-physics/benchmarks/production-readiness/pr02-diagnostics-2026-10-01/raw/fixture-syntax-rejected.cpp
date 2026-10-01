// PR02 linked public diagnostics, separate from timing and whole-engine physics.
#include "box3d/box3d.h"
#include "native_api_status.h"
#ifndef DIAGNOSTIC_BASELINE
#include "native_diagnostics.h"
#endif
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
extern "C" void gpu_samples_get_cpu_stats(b3WorldId, b3Profile*, b3Counters*);
#endif

static b3BodyId box(b3WorldId world, b3BodyType type, b3Vec3 position, b3ShapeId* shape=nullptr) {
    b3BodyDef def=b3DefaultBodyDef(); def.type=type; def.position=position;
    b3BodyId body=b3CreateBody(world,&def);
    b3ShapeDef sd=b3DefaultShapeDef(); b3Hull hull=b3MakeBoxHull(0.5f,0.5f,0.5f);
    b3ShapeId id=b3CreateHullShape(body,&sd,&hull);
    if(shape) *shape=id;
    return body;
}
#ifndef DIAGNOSTIC_BASELINE
struct Bounds { b3WorldId world; std::vector<GpuNativeShapeBounds> records; };
static void collect(const GpuNativeShapeBounds* record,void* context) {
    Bounds& bounds=*static_cast<Bounds*>(context);
    // Visitor is outside the world lock, so a reentrant read is safe.
    assert(b3World_IsValid(bounds.world));
    bounds.records.push_back(*record);
}
static void invalid(const char* name,void (*call)(b3WorldId),b3WorldId id) {
    gpu_b3_native_api_clear_error();call(id);
    assert(errno==EINVAL && std::strcmp(gpu_b3_native_api_last_error(),name)==0);
}
#endif

int main() {
    b3WorldDef wd=b3DefaultWorldDef();wd.gravity={0,0,0};wd.enableSleep=false;
    b3WorldId world=b3CreateWorld(&wd);assert(b3World_IsValid(world));
    b3ShapeId shape={};box(world,b3_staticBody,{0,0,0});
    b3BodyId body=box(world,b3_dynamicBody,{0,0.99f,0},&shape);
    box(world,b3_dynamicBody,{5,2,0});box(world,b3_kinematicBody,{10,2,0});
    for(int i=0;i<4;++i)b3World_Step(world,1.0f/60.0f,4);
    b3Counters counters=b3World_GetCounters(world);
    b3Capacity peak=b3World_GetMaxCapacity(world);
    b3Profile profile=b3World_GetProfile(world);
    b3ContactData data[8]={};int contacts=b3Body_GetContactData(body,data,8);
    assert(contacts==1);
#ifdef DIAGNOSTIC_BASELINE
    assert(counters.contactCount==0);
    assert(peak.staticBodyCount==0 && peak.dynamicBodyCount==0 && peak.contactCount==0);
    const float* values=reinterpret_cast<const float*>(&profile);
    for(unsigned i=0;i<sizeof profile/sizeof(float);++i) assert(values[i]==0.0f);
    std::puts("baseline reproduced: one real public contact; counter/profile/capacity placeholders");
#else
    assert(counters.bodyCount==4 && counters.shapeCount==4 && counters.jointCount==0);
    assert(counters.contactCount==contacts && counters.islandCount==2);
    int touching=0;for(int count:counters.manifoldCounts)touching+=count;
    assert(touching==1);
    assert(counters.stackUsed==-1 && counters.taskCount==-1);
    for(int count:counters.colorCounts)assert(count==-1);
    assert(peak.staticBodyCount==1 && peak.dynamicBodyCount==3);
    assert(peak.staticShapeCount==1 && peak.dynamicShapeCount==3 && peak.contactCount==1);
    GpuNativeProfile extended={};gpu_b3_world_native_profile(world,&extended);
    assert(extended.valid==GPU_DIAGNOSTIC_OK && extended.physics_step==4);
    assert(extended.timestamp_step<=extended.physics_step);
    const float* values=reinterpret_cast<const float*>(&profile);
    for(unsigned i=0;i<23;++i) {
        if(extended.measured_fields & (1u<<i))assert(std::isfinite(values[i]) && values[i]>=0);
        else assert(std::isnan(values[i]));
    }
    GpuNativeAllocation allocation={};gpu_b3_world_native_allocation(world,&allocation);
    assert(allocation.valid==GPU_DIAGNOSTIC_OK && allocation.primary_buffer_bytes>0);
    assert(allocation.primary_buffer_bytes==static_cast<unsigned>(counters.byteCount));
    assert(allocation.body_capacity>=4 && allocation.shape_capacity>=4);
    Bounds bounds{world,{}};
    assert(gpu_b3_world_native_visit_shape_bounds(world,b3_dynamicBody,collect,&bounds)==GPU_DIAGNOSTIC_OK);
    assert(bounds.records.size()==2);
    const b3AABB expected=b3Shape_GetAABB(shape);
    bool found=false;
    for(const auto& record:bounds.records)if(record.shape.index1==shape.index1) {
        found=true;assert(record.body.index1==body.index1);
        assert(std::memcmp(&record.bounds,&expected,sizeof expected)==0);
    }
    assert(found);
    b3World_DumpMemoryStats(world);b3World_DumpShapeBounds(world,b3_dynamicBody);
    assert(b3Shape_GetType(shape)==b3_hullShape);
    b3Sphere sphere={{0,0,0},0.5f};b3Shape_SetSphere(shape,&sphere);
    assert(b3Shape_GetType(shape)==b3_sphereShape);
#ifdef GPU_API_DUAL
    b3Profile cpu_profile={};b3Counters cpu_counters={};
    gpu_samples_get_cpu_stats(world,&cpu_profile,&cpu_counters);
    assert(cpu_counters.bodyCount==4 && cpu_counters.shapeCount==4);
    assert(cpu_counters.stackUsed>=0 && cpu_counters.taskCount>=0);
    assert(cpu_profile.step>=0 && std::isfinite(cpu_profile.step));
#endif
    b3WorldId other=b3CreateWorld(&wd);
    assert(b3World_GetCounters(other).bodyCount==0);
    assert(b3World_GetMaxCapacity(other).dynamicBodyCount==0);
    b3DestroyWorld(other);
    b3DestroyBody(body);
    assert(b3World_GetCounters(world).contactCount==0);
    assert(b3World_GetCounters(world).islandCount==-1);
    assert(b3World_GetMaxCapacity(world).contactCount==1);
#endif
    b3DestroyWorld(world);
#ifndef DIAGNOSTIC_BASELINE
    gpu_b3_native_api_clear_error();counters=b3World_GetCounters(world);
    assert(counters.bodyCount==-1 && counters.contactCount==-1);
    assert(errno==EINVAL && std::strcmp(gpu_b3_native_api_last_error(),"b3World_GetCounters")==0);
    invalid("b3World_DumpMemoryStats",b3World_DumpMemoryStats,world);
    gpu_b3_native_api_clear_error();b3World_DumpShapeBounds(world,b3_dynamicBody);
    assert(errno==EINVAL && std::strcmp(gpu_b3_native_api_last_error(),"b3World_DumpShapeBounds")==0);
    gpu_b3_native_api_clear_error();
    assert(gpu_b3_native_api_last_error()==nullptr && errno==0);
    std::puts("native diagnostics C contract passed: counters/profile/peak/bytes/bounds/type/ownership/stale IDs");
#endif
}
