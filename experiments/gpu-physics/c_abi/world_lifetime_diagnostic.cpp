// Source-faithful world lifetime diagnosis. Never call invalid CPU mutators.
#include "box3d/box3d.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
extern "C" { void SetSelectedBody(b3BodyId){} void SetComparisonSelectedBody(b3BodyId){} }
static int observations=0, mismatches=0;
static void check(const char* phase,const char* field,uint64_t got,uint64_t expected) {
    bool match=got==expected;
    std::printf("{\"phase\":\"%s\",\"field\":\"%s\",\"got\":%llu,\"expected\":%llu,\"match\":%s}\n",phase,field,(unsigned long long)got,(unsigned long long)expected,match?"true":"false");
    ++observations; if(!match)++mismatches;
}
int main() {
    b3WorldDef def=b3DefaultWorldDef();
    b3WorldId old=b3CreateWorld(&def);assert(b3World_IsValid(old));
    b3World_SetUserData(old,(void*)uintptr_t(11));
    b3WorldId other=b3CreateWorld(&def);assert(b3World_IsValid(other));
    b3World_SetUserData(other,(void*)uintptr_t(33));
    b3DestroyWorld(old);
    check("destroy","old-invalid",b3World_IsValid(old),false);
    check("destroy","other-valid",b3World_IsValid(other),true);
    b3WorldId fresh=b3CreateWorld(&def);assert(b3World_IsValid(fresh));
    b3World_SetUserData(fresh,(void*)uintptr_t(22));
    std::printf("{\"ids\":true,\"old\":[%u,%u],\"fresh\":[%u,%u],\"other\":[%u,%u]}\n",old.index1,old.generation,fresh.index1,fresh.generation,other.index1,other.generation);
    check("recreate","distinct-world-id",b3StoreWorldId(fresh)!=b3StoreWorldId(old),true);
    check("recreate","old-stays-invalid",b3World_IsValid(old),false);
    check("recreate","fresh-valid",b3World_IsValid(fresh),true);
    check("recreate","fresh-user-data",uintptr_t(b3World_GetUserData(fresh)),22);
#ifdef GPU_WORLD_DIAGNOSTIC
    check("stale-read","old-cannot-read-fresh-user-data",uintptr_t(b3World_GetUserData(old)),0);
    b3World_EnableWarmStarting(old,false);
#endif
    check("stale-write","fresh-warm-start-preserved",b3World_IsWarmStartingEnabled(fresh),true);
#ifdef GPU_WORLD_DIAGNOSTIC
    b3DestroyWorld(old);
#endif
    check("stale-destroy","fresh-stays-valid",b3World_IsValid(fresh),true);
    check("stale-destroy","other-stays-valid",b3World_IsValid(other),true);
    check("stale-destroy","other-user-data-preserved",uintptr_t(b3World_GetUserData(other)),33);
    // Combined cleanup must also remove a mapped CPU world if GPU was lost.
    b3DestroyWorld(fresh);
    b3DestroyWorld(other);
    b3WorldId live=b3CreateWorld(&def);assert(b3World_IsValid(live));
    b3WorldId forged=live;forged.generation=uint16_t(forged.generation+1);
    check("wrong-generation","forged-invalid",b3World_IsValid(forged),false);
#ifdef GPU_WORLD_DIAGNOSTIC
    b3DestroyWorld(forged);
#endif
    check("wrong-generation","live-stays-valid",b3World_IsValid(live),true);
    b3DestroyWorld(live);
    std::printf("{\"summary\":true,\"observations\":%d,\"mismatches\":%d}\n",observations,mismatches);
#ifndef GPU_WORLD_DIAGNOSTIC
    assert(mismatches==0);
#endif
}
