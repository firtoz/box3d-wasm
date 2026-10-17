// PR02 regression for defaults outside the stateful-symbol inventory.
#include "box3d/box3d.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#ifdef GPU_API_DUAL
extern "C" void SetSelectedBody(b3BodyId) {}
extern "C" void SetComparisonSelectedBody(b3BodyId) {}
#endif

struct Hits { b3ShapeId expected; int count = 0; };
static bool overlap(b3ShapeId id, void* raw) {
    auto& hits = *static_cast<Hits*>(raw);
    assert(B3_ID_EQUALS(id, hits.expected));
    ++hits.count;
    return true;
}
static int checks = 0;
static void query(b3WorldId world, b3ShapeId shape, uint64_t mask, bool expected) {
    auto filter = b3DefaultQueryFilter();
    filter.maskBits = mask;
    auto ray = b3World_CastRayClosest(world, {-2, 0, 0}, {4, 0, 0}, filter);
    assert(ray.hit == expected);
    if (ray.hit) assert(B3_ID_EQUALS(ray.shapeId, shape));
    Hits hits{shape};
    b3World_OverlapAABB(world, {{-1, -1, -1}, {1, 1, 1}}, filter, overlap, &hits);
    assert(hits.count == int(expected));
    checks += 2;
}
int main() {
    const auto filter = b3DefaultFilter();
    const auto shapeDefault = b3DefaultShapeDef();
    const auto queryDefault = b3DefaultQueryFilter();
    assert(filter.categoryBits == UINT64_MAX && filter.maskBits == UINT64_MAX && filter.groupIndex == 0);
    assert(shapeDefault.filter.categoryBits == filter.categoryBits);
    assert(shapeDefault.filter.maskBits == filter.maskBits && shapeDefault.filter.groupIndex == 0);
    assert(queryDefault.categoryBits == UINT64_MAX && queryDefault.maskBits == UINT64_MAX);
    assert(queryDefault.id == 0 && queryDefault.name == nullptr);
    auto wd = b3DefaultWorldDef();
    const auto world = b3CreateWorld(&wd);
    const auto other = b3CreateWorld(&wd);
    auto bd = b3DefaultBodyDef();
    auto body = b3CreateBody(world, &bd);
    auto otherBody = b3CreateBody(other, &bd);
    auto sd = shapeDefault;
    sd.filter = filter; // Exercise the alias, not just the shape definition.
    const b3Sphere sphere{{0, 0, 0}, 0.5f};
    const auto shape = b3CreateSphereShape(body, &sd, &sphere);
    const auto otherShape = b3CreateSphereShape(otherBody, &sd, &sphere);
    for (uint64_t mask : {uint64_t(0), uint64_t(1), uint64_t(2), uint64_t(1) << 63, UINT64_MAX}) {
        query(world, shape, mask, mask != 0);
        query(other, otherShape, mask, mask != 0);
    }
    auto changed = filter;
    changed.categoryBits = 2;
    b3Shape_SetFilter(shape, changed, false);
    const auto read = b3Shape_GetFilter(shape);
    assert(read.categoryBits == 2 && read.maskBits == UINT64_MAX && read.groupIndex == 0);
    query(world, shape, 1, false);
    query(world, shape, 2, true);
    query(world, shape, uint64_t(1) << 63, false);
    query(other, otherShape, uint64_t(1) << 63, true);
    b3DestroyShape(shape, false);
    assert(!b3Shape_IsValid(shape));
    query(world, shape, UINT64_MAX, false);
    const auto replacement = b3CreateSphereShape(body, &sd, &sphere);
    assert(!B3_ID_EQUALS(replacement, shape) && !b3Shape_IsValid(shape));
    query(world, replacement, uint64_t(1) << 63, true);
    b3DestroyWorld(world);
    assert(!b3World_IsValid(world) && b3World_IsValid(other));
    query(other, otherShape, 2, true);
    b3DestroyWorld(other);
    std::printf("PASS default filters; category=%llu mask=%llu group=%d; %d query/identity checks; independent worlds, mutation, replacement and stale IDs\n",
        (unsigned long long)filter.categoryBits, (unsigned long long)filter.maskBits, filter.groupIndex, checks);
}
