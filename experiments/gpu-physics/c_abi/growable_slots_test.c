#include "growable_slots.h"
#include "both_ids.h"
#include <assert.h>

int main(void)
{
    GpuSlots slots = {0};
    uint64_t* first = (uint64_t*)gpu_slots_get(&slots, 1, sizeof(uint64_t), true);
    *first = UINT64_C(0xfeed12345678);
    for (int32_t i = 2; i <= 100001; ++i)
        *(uint64_t*)gpu_slots_get(&slots, i, sizeof(uint64_t), true) = (uint64_t)i;
    assert(first == gpu_slots_get(&slots, 1, sizeof(uint64_t), false));
    assert(*first == UINT64_C(0xfeed12345678));
    for (int32_t i = 2; i <= 100001; ++i)
        assert(*(uint64_t*)gpu_slots_get(&slots, i, sizeof(uint64_t), false) == (uint64_t)i);
    assert(!gpu_slots_get(&slots, 1000000, sizeof(uint64_t), false));
    assert(!gpu_slots_get(&slots, -1, sizeof(uint64_t), true));
    gpu_slots_release(&slots);
    assert(!slots.chunks && !slots.high_water);

    b3WorldId w1 = {1, 3}, w2 = {2, 4};
    both_map_world(w1, (b3WorldId){7, 8});
    both_map_world(w2, (b3WorldId){8, 9});
    for (int32_t i = 1; i <= 100001; ++i)
    {
        both_map_body((b3BodyId){i, 1, 3}, (b3BodyId){i + 200000, 7, 8});
        both_map_shape((b3ShapeId){i, 1, 3}, (b3ShapeId){i + 300000, 7, 8});
        both_map_joint((b3JointId){i, 1, 3}, (b3JointId){i + 400000, 7, 8});
        both_map_body((b3BodyId){i, 2, 4}, (b3BodyId){i + 500000, 8, 9});
    }
    for (int32_t i = 1; i <= 100001; ++i)
    {
        assert(both_cpu_body((b3BodyId){i, 1, 3}).index1 == i + 200000);
        assert(both_cpu_shape((b3ShapeId){i, 1, 3}).index1 == i + 300000);
        assert(both_cpu_joint((b3JointId){i, 1, 3}).index1 == i + 400000);
        assert(both_cpu_body((b3BodyId){i, 2, 4}).index1 == i + 500000);
    }
    b3ShapeId old = {100001, 1, 3}, fresh = {100001, 1, 4};
    both_unmap_shape(old);
    assert(!both_has_cpu_shape(old));
    both_map_shape(fresh, (b3ShapeId){700000, 7, 9});
    both_unmap_shape(old); // Stale handles cannot clear a reused slot.
    assert(!both_has_cpu_shape(old) && both_has_cpu_shape(fresh));
    assert(both_cpu_shape(fresh).index1 == 700000);
    both_unmap_world((b3WorldId){1, 2});
    assert(both_has_cpu_shape(fresh));
    both_unmap_world(w1);
    assert(!both_has_cpu_shape(fresh));
    assert(!both_has_cpu_body((b3BodyId){100001, 1, 3}));
    assert(both_has_cpu_body((b3BodyId){100001, 2, 4}));
    both_clear_maps();
    assert(!both_has_cpu_body((b3BodyId){100001, 2, 4}));
    puts("growable metadata: 100001 slots, stable pointers, independent worlds, reuse and cleanup passed");
}
