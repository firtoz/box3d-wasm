// Deliberately retire only the GPU object. The combined adapter still owns a
// genuine CPU world, so public cleanup must release it and all its metadata.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include "both_ids.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
extern "C" {
void gpu_b3_destroy_world(b3WorldId);
bool cpu_b3World_IsValid(b3WorldId);
int gpu_shape_geometry_mirror_count(void);
void SetSelectedBody(b3BodyId){} void SetComparisonSelectedBody(b3BodyId){}
}
static int observations=0,mismatches=0;
static void check(const char* phase,const char* field,uint64_t got,uint64_t expected) {
    bool match=got==expected;
    std::printf("{\"phase\":\"%s\",\"field\":\"%s\",\"got\":%llu,\"expected\":%llu,\"match\":%s}\n",phase,field,(unsigned long long)got,(unsigned long long)expected,match?"true":"false");
    ++observations;if(!match)++mismatches;
}
static b3ShapeId shape(b3WorldId w,const b3CompoundData* geometry) {
    auto bd=b3DefaultBodyDef();auto sd=b3DefaultShapeDef();sd.userData=(void*)uintptr_t(321);
    return b3CreateBakedCompoundShape(b3CreateBody(w,&bd),&sd,geometry);
}
int main() {
    auto box=b3MakeCubeHull(.5f);b3CompoundHullDef child{};child.hull=&box.base;
    child.transform=b3Transform_identity;child.material=b3DefaultSurfaceMaterial();
    b3CompoundDef cd{};cd.hulls=&child;cd.hullCount=1;auto* geometry=b3CreateCompound(&cd);assert(geometry);
    auto def=b3DefaultWorldDef();auto world=b3CreateWorld(&def);auto parent=shape(world,geometry);auto cpu=both_cpu_world(world);
    check("initial","gpu-valid",b3World_IsValid(world),true);
    check("initial","cpu-valid",cpu_b3World_IsValid(cpu),true);
    check("initial","cpu-mapped",both_has_cpu_world(world),true);
    check("initial","one-owned-mirror",gpu_shape_geometry_mirror_count(),1);
    gpu_b3_destroy_world(world);
    check("gpu-retired","gpu-invalid",b3World_IsValid(world),false);
    check("gpu-retired","cpu-still-valid",cpu_b3World_IsValid(cpu),true);
    check("gpu-retired","mapping-still-owned",both_has_cpu_world(world),true);
    check("gpu-retired","geometry-still-owned",gpu_shape_geometry_mirror_count(),1);
    b3DestroyWorld(world);
    check("public-cleanup","cpu-invalid",cpu_b3World_IsValid(cpu),false);
    check("public-cleanup","cpu-unmapped",both_has_cpu_world(world),false);
    check("public-cleanup","geometry-released",gpu_shape_geometry_mirror_count(),0);
    auto fresh=b3CreateWorld(&def);auto live=shape(fresh,geometry);
    check("recreate","distinct-root",b3StoreWorldId(fresh)!=b3StoreWorldId(world),true);
    check("recreate","distinct-child",b3StoreShapeId(live)!=b3StoreShapeId(parent),true);
    check("recreate","live-child-valid",b3Shape_IsValid(live),true);
    check("recreate","stale-child-invalid",b3Shape_IsValid(parent),false);
    check("recreate","live-user-data",uintptr_t(b3Shape_GetUserData(live)),321);
    check("recreate","fresh-cpu-valid",cpu_b3World_IsValid(both_cpu_world(fresh)),true);
    b3DestroyWorld(world);
    check("stale-cleanup","fresh-cpu-valid",cpu_b3World_IsValid(both_cpu_world(fresh)),true);
    check("stale-cleanup","fresh-geometry-preserved",gpu_shape_geometry_mirror_count(),1);
    auto freshCpu=both_cpu_world(fresh);b3DestroyWorld(fresh);
    check("final-cleanup","cpu-invalid",cpu_b3World_IsValid(freshCpu),false);
    check("final-cleanup","geometry-released",gpu_shape_geometry_mirror_count(),0);
    b3DestroyCompound(geometry);
    std::printf("{\"summary\":true,\"observations\":%d,\"mismatches\":%d}\n",observations,mismatches);return mismatches?1:0;
}
