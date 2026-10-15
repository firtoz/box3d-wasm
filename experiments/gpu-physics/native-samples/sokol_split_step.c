#include "sokol_split_step.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StepDraw {
    void (*replay)(const void*);
    void* data;
    size_t capacity;
} StepDraw;
static StepDraw* draws;
static size_t count, capacity;
static unsigned world_depth;
static bool recording;
static int ui_width;

static void* checked_realloc(void* p, size_t bytes) {
    void* result = realloc(p, bytes);
    if (!result) {
        fputs("[split-step] drawing journal allocation failed\n", stderr);
        abort();
    }
    return result;
}
void split_step_begin(bool enabled) {
    assert(world_depth == 0);
    recording = enabled;
    count = 0;
}
void split_step_end(void) {
    assert(world_depth == 0);
    recording = false;
}
void split_step_world_begin(void) { ++world_depth; }
void split_step_world_end(void) { assert(world_depth); --world_depth; }
void split_step_record(void (*replay)(const void*), const void* data, size_t size) {
    if (!recording || world_depth) return;
    assert(replay && data && size);
    if (count == capacity) {
        if (capacity > SIZE_MAX / 2 / sizeof(*draws)) abort();
        size_t next = capacity ? capacity * 2 : 64;
        draws = checked_realloc(draws, next * sizeof(*draws));
        memset(draws + capacity, 0, (next - capacity) * sizeof(*draws));
        capacity = next;
    }
    StepDraw* d = &draws[count++];
    if (d->capacity < size) {
        d->data = checked_realloc(d->data, size);
        d->capacity = size;
    }
    memcpy(d->data, data, size);
    d->replay = replay;
}
void split_step_replay(void) {
    assert(!recording && world_depth == 0);
    for (size_t i = 0; i < count; ++i) draws[i].replay(draws[i].data);
}
void split_step_ui_begin(int pane_width) { ui_width = pane_width; }
void split_step_ui_end(void) { ui_width = 0; }
int split_step_ui_width(void) { return ui_width; }
void split_step_shutdown(void) {
    for (size_t i = 0; i < capacity; ++i) free(draws[i].data);
    free(draws);
    draws = NULL;
    count = capacity = 0;
    world_depth = 0;
    recording = false;
    ui_width = 0;
}
