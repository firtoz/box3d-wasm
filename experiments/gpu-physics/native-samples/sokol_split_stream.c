// Two independent instance streams, one GL submission/present. Immutable geometry
// and render targets are shared; a stream is never updated twice in one frame.
#include "sokol_split_stream.h"
#include <assert.h>
#include <stdlib.h>
static int current_view;
typedef struct BufferCopy { sg_buffer original, copy; struct BufferCopy* next; } BufferCopy;
typedef struct ViewCopy { sg_view original, copy; struct ViewCopy* next; } ViewCopy;
static BufferCopy* buffers;
static ViewCopy* views;
void split_stream_view(int view) { current_view = view; }
static sg_buffer remap_buffer(sg_buffer original) {
    if (current_view != 1 || !original.id) return original;
    for (BufferCopy* b = buffers; b; b = b->next)
        if (b->original.id == original.id) return b->copy;
    return original;
}
void split_update_buffer(sg_buffer original, const sg_range* data) {
    sg_buffer target = remap_buffer(original);
    if (current_view == 1 && target.id == original.id) {
        sg_buffer_desc desc = sg_query_buffer_desc(original);
        desc.data = (sg_range){0};
        target = sg_make_buffer(&desc);
        BufferCopy* b = calloc(1, sizeof(*b));
        assert(b); *b = (BufferCopy){original, target, buffers}; buffers = b;
    }
    sg_update_buffer(target, data);
}
void split_apply_bindings(const sg_bindings* original) {
    if (current_view != 1) { sg_apply_bindings(original); return; }
    sg_bindings b = *original;
    for (int i = 0; i < SG_MAX_VERTEXBUFFER_BINDSLOTS; ++i) b.vertex_buffers[i] = remap_buffer(b.vertex_buffers[i]);
    b.index_buffer = remap_buffer(b.index_buffer);
    for (int i = 0; i < SG_MAX_VIEW_BINDSLOTS; ++i) {
        if (!b.views[i].id) continue;
        sg_view_desc desc = sg_query_view_desc(b.views[i]);
        if (!desc.storage_buffer.buffer.id) continue;
        sg_buffer mapped = remap_buffer(desc.storage_buffer.buffer);
        if (mapped.id == desc.storage_buffer.buffer.id) continue;
        ViewCopy* v = views;
        while (v && v->original.id != b.views[i].id) v = v->next;
        if (!v) {
            desc.storage_buffer.buffer = mapped;
            v = calloc(1, sizeof(*v)); assert(v);
            *v = (ViewCopy){b.views[i], sg_make_view(&desc), views}; views = v;
        }
        b.views[i] = v->copy;
    }
    sg_apply_bindings(&b);
}
void split_destroy_buffer(sg_buffer original) {
    for (BufferCopy** p = &buffers; *p; p = &(*p)->next) {
        if ((*p)->original.id == original.id) {
            BufferCopy* b = *p; *p = b->next; sg_destroy_buffer(b->copy); free(b); break;
        }
    }
    sg_destroy_buffer(original);
}
void split_destroy_view(sg_view original) {
    for (ViewCopy** p = &views; *p; p = &(*p)->next) {
        if ((*p)->original.id == original.id) {
            ViewCopy* v = *p; *p = v->next; sg_destroy_view(v->copy); free(v); break;
        }
    }
    sg_destroy_view(original);
}
