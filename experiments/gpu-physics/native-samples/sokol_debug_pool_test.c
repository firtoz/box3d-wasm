// Include the actual generated adapter to exercise its static allocation and
// compound operations, not a rewritten model of the allocator.
#include "debug_adapter.c"
#ifndef EXPECT_FIXED
#define GetDebugShape(i) (&s_adapter.pool[(i)])
#endif
#include <limits.h>

int main(int argc, char** argv)
{
    assert(argc == 2);
    int scenario = atoi(argv[1]);
    InitAdapter();
    const int reservation = sample_renderer_capacity();
    int parentIndex = AllocDebugShape();
    DebugShape* parent = GetDebugShape(parentIndex);
    parent->kind = Box3DUS_Compound;
    parent->shapeId = (b3ShapeId){17, 1, 9};
    parent->bodyId = (b3BodyId){13, 1, 7};
    parent->bodyType = b3_staticBody;
    parent->isGround = true;
    parent->compound.firstChild = -1;
    b3ChildShape child = {0};
    child.type = b3_sphereShape;
    child.sphere = (b3Sphere){{1,2,3}, 0.5f};
    child.transform = b3Transform_identity;
    int count = scenario == 0 ? 105004 : reservation * 3 + 3;
    parent->compound.childMap = malloc((size_t)count * sizeof(int));
    parent->compound.childMapCount = count;
    int prev = -1;
    int registered = 0, firstMissing = -1;
    for (int i = 0; i < count; ++i)
    {
        int slot = CreateCompoundChild(&child, parent);
        if (slot < 0)
        {
            if (firstMissing < 0) firstMissing = i;
            parent->compound.childMap[i] = -1;
            continue;
        }
        ++registered;
        parent->compound.childMap[i] = slot;
        if (prev < 0) parent->compound.firstChild = slot;
        else GetDebugShape(prev)->nextChild = slot;
        prev = slot;
        assert(GetDebugShape(parentIndex) == parent);
#ifdef EXPECT_FIXED
        assert(GetDebugShape(slot)->poolIndex == slot);
#endif
        assert(B3_ID_EQUALS(GetDebugShape(slot)->shapeId, parent->shapeId));
        assert(B3_ID_EQUALS(GetDebugShape(slot)->bodyId, parent->bodyId));
        assert(GetDebugShape(slot)->isGround && GetDebugShape(slot)->bodyType == b3_staticBody);
        assert(GetDebugShape(slot)->sphere.center.y == 2 && GetDebugShape(slot)->sphere.radius == .5f);
    }
    assert(GetDebugShapeCount() == registered + 1);
#ifdef EXPECT_FIXED
    assert(registered == count && firstMissing == -1 && s_adapter.poolCapacity >= count + 1);
#else
    assert(registered == reservation - 1 && firstMissing == reservation - 1);
#endif
    printf("calculation inputs reservation=%d children=%d; registered=%d firstMissing=%d\n", reservation,count,registered,firstMissing);
    // Exercise the exact index lookup used by culling and selected-body highlight.
    s_adapter.selectedBodyId = parent->bodyId;
    for (int i = 0; i < count; ++i)
    {
        if (parent->compound.childMap[i] < 0) continue;
        DebugShape* c = GetDebugShape(parent->compound.childMap[i]);
        assert(ResolveHighlightKind(c->bodyId, c->shapeId) == HIGHLIGHT_KIND_SELECT);
    }
    if (scenario == 1 || scenario == 4)
    {
        ResetAdapterPool();
        assert(GetDebugShapeCount() == 0);
        DestroyDebugShape(parent, NULL); // stale post-reset callback must stay inert
        assert(GetDebugShapeCount() == 0);
    }
    else
    {
        DestroyDebugShape(parent, NULL);
        assert(GetDebugShapeCount() == 0);
    }
    if (scenario == 2 || scenario == 4)
    {
        int slot = AllocDebugShape();
        DebugShape* fresh = GetDebugShape(slot);
        fresh->kind = Box3DUS_Sphere;
        fresh->shapeId = (b3ShapeId){17, 1, 10};
        fresh->bodyId = (b3BodyId){13, 1, 8};
#ifdef EXPECT_FIXED
        assert(fresh->poolIndex == slot);
#endif
        FreeDebugShape(slot);
        FreeDebugShape(slot); // double-free must not create a free-list cycle
        assert(GetDebugShapeCount() == 0);
        int a = AllocDebugShape(), b = AllocDebugShape();
        assert(a != b);
        GetDebugShape(a)->kind = GetDebugShape(b)->kind = Box3DUS_Sphere;
    }
    sample_renderer_release_adapter();
#ifdef EXPECT_FIXED
    assert(s_adapter.poolCapacity == 0 && !s_adapter.pool.chunks);
#else
    assert(s_adapter.pool == NULL);
#endif
    if (scenario == 3 || scenario == 5)
    {
        InitAdapter();
        assert(GetDebugShapeCount() == 0);
        int slot = AllocDebugShape();
        GetDebugShape(slot)->kind = Box3DUS_Sphere;
        assert(slot == 0);
        sample_renderer_release_adapter();
    }
    printf("actual adapter case %d: allocation contract, stable addresses, identity/highlight, cleanup pass\n", scenario);
}
