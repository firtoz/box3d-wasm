#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Direct-index metadata with stable addresses, including during reentrant debug
// callbacks. Only the pointer directory moves; allocated chunks never move.
#define GPU_SLOT_CHUNK 256u
#define GPU_METADATA_WORLDS 64u

typedef struct GpuSlots
{
    void** chunks;
    size_t chunk_capacity;
    uint32_t high_water; // exclusive; includes unallocated holes
} GpuSlots;

static inline void gpu_slots_oom(void)
{
    fputs("GPU native metadata allocation failed\n", stderr);
    abort(); // Never silently drop geometry or an independent CPU mapping.
}

static inline void* gpu_slots_get(GpuSlots* slots, int32_t index, size_t stride, bool create)
{
    if (index <= 0) return NULL;
    size_t chunk = (uint32_t)index / GPU_SLOT_CHUNK;
    if (chunk >= slots->chunk_capacity)
    {
        if (!create) return NULL;
        size_t capacity = slots->chunk_capacity ? slots->chunk_capacity : 4;
        while (capacity <= chunk) capacity *= 2;
        if (capacity > SIZE_MAX / sizeof(void*)) gpu_slots_oom();
        void** grown = (void**)realloc(slots->chunks, capacity * sizeof(void*));
        if (!grown) gpu_slots_oom();
        memset(grown + slots->chunk_capacity, 0, (capacity - slots->chunk_capacity) * sizeof(void*));
        slots->chunks = grown;
        slots->chunk_capacity = capacity;
    }
    if (!slots->chunks[chunk])
    {
        if (!create) return NULL;
        if (!stride || stride > SIZE_MAX / GPU_SLOT_CHUNK) gpu_slots_oom();
        slots->chunks[chunk] = calloc(GPU_SLOT_CHUNK, stride);
        if (!slots->chunks[chunk]) gpu_slots_oom();
    }
    if (create && (uint32_t)index >= slots->high_water) slots->high_water = (uint32_t)index + 1;
    return (char*)slots->chunks[chunk] + ((uint32_t)index % GPU_SLOT_CHUNK) * stride;
}

static inline void gpu_slots_release(GpuSlots* slots)
{
    for (size_t i = 0; i < slots->chunk_capacity; ++i) free(slots->chunks[i]);
    free(slots->chunks);
    memset(slots, 0, sizeof(*slots));
}
