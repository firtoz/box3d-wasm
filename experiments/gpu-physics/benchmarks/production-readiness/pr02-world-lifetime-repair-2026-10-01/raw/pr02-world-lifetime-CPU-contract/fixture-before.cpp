// Public destruction must reject stale identities before releasing replacement
// geometry. CPU controls never receive invalid mutator/destructor calls.
#include "box3d/box3d.h"
#include "box3d/collision.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#ifdef GPU_CLEANUP_CONTRACT
extern "C" { int gpu_shape_geometry_mirror_count(void); }
#endif
extern "C" { void SetSelectedBody(b3BodyId){} void SetComparisonSelectedBody(b3BodyId){} }
static int observations=0,mismatches=0;
static void check(const char* phase,const char* field,uint64_t got,uint64_t expected) {
    bool match=got==expected;
    std::printf("{\"phase\":\"%s\",\"field\":\"%s\",\"got\":%llu,\"expected\":%llu,\"match\":%s}\n",phase,field,(unsigned long long)got,(unsigned long long)expected,match?"true":"false");
    ++observations;if(!match)++mismatches;
}
static b3ShapeId make_compound(b3WorldId world,const b3CompoundData* compound) {
    auto bd=b3DefaultBodyDef();auto body=b3CreateBody(world,&bd);
    auto sd=b3DefaultShapeDef();sd.userData=(void*)uintptr_t(222);
    return b3CreateBakedCompoundShape(body,&sd,compound);
}
int main() {
    auto box=b3MakeBoxHull(.5f,.5f,.5f);b3CompoundHullDef hulls[2]{};
    for(int i=0;i<2;++i){hulls[i].hull=&box.base;hulls[i].material=b3DefaultSurfaceMaterial();hulls[i].transform={{float(i)*2.f,0,0},b3Quat_identity};}
    b3CompoundDef cd{};cd.hulls=hulls;cd.hullCount=2;auto* compound=b3CreateCompound(&cd);assert(compound);
    auto wd=b3DefaultWorldDef();auto old=b3CreateWorld(&wd);auto stale=make_compound(old,compound);
    b3DestroyWorld(old);auto fresh=b3CreateWorld(&wd);auto live=make_compound(fresh,compound);
    auto other=b3CreateWorld(&wd);auto isolated=make_compound(other,compound);
    check("recreate","distinct-world-id",b3StoreWorldId(old)!=b3StoreWorldId(fresh),true);
    check("recreate","old-invalid",b3World_IsValid(old),false);
    check("recreate","stale-shape-invalid",b3Shape_IsValid(stale),false);
#ifdef GPU_CLEANUP_CONTRACT
    int mirrors=gpu_shape_geometry_mirror_count();check("recreate","four-owned-hull-mirrors",mirrors,4);
    b3DestroyWorld(old);
    check("stale-world","owned-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    auto forged=fresh;forged.generation=uint16_t(forged.generation+1);
    b3DestroyWorld(forged);
    check("forged-world","owned-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    b3DestroyShape(stale,false);
    check("stale-parent","owned-child-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
    auto wrong=live;wrong.generation=uint16_t(wrong.generation+1);b3DestroyShape(wrong,false);
    check("forged-parent","owned-child-geometry-preserved",gpu_shape_geometry_mirror_count(),mirrors);
#endif
    check("after-invalid","live-world-valid",b3World_IsValid(fresh),true);
    check("after-invalid","live-shape-valid",b3Shape_IsValid(live),true);
    check("after-invalid","live-user-data",uintptr_t(b3Shape_GetUserData(live)),222);
    check("after-invalid","other-shape-valid",b3Shape_IsValid(isolated),true);
    check("after-invalid","other-user-data",uintptr_t(b3Shape_GetUserData(isolated)),222);
    b3DestroyShape(live,false);check("valid-destroy","live-shape-invalid",b3Shape_IsValid(live),false);
#ifdef GPU_CLEANUP_CONTRACT
    check("valid-destroy","isolated-mirrors-only",gpu_shape_geometry_mirror_count(),2);
#endif
    check("valid-destroy","other-world-valid",b3World_IsValid(other),true);
    b3DestroyWorld(fresh);b3DestroyWorld(other);b3DestroyCompound(compound);
#ifdef GPU_CLEANUP_CONTRACT
    check("cleanup","no-owned-mirrors",gpu_shape_geometry_mirror_count(),0);
#endif
    std::printf("{\"summary\":true,\"observations\":%d,\"mismatches\":%d}\n",observations,mismatches);
    return mismatches?1:0;
}
