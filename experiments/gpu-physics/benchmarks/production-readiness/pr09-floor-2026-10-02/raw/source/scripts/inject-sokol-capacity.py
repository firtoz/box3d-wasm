#!/usr/bin/env python3
"""Generate local renderer reservation/measurement hooks; keep upstream clean."""
from pathlib import Path
import sys

def replace(text, old, new):
    assert text.count(old) == 1, f'upstream renderer changed: {old!r}'
    return text.replace(old, new)

def generate(src, dst):
    dst.mkdir(parents=True, exist_ok=True)
    for path in src.glob('*.c'):
        if path.name == 'sokol_impl.c':
            continue
        text = path.read_text()
        if path.name == 'debug_adapter.c':
            text = '#include <limits.h>\n#include "../c_abi/growable_slots.h"\n' + text
            text = replace(text, '#define BOX3D_USER_SHAPE_CAPACITY 65536', '#define BOX3D_USER_SHAPE_CAPACITY (sample_renderer_capacity())')
            text = replace(text, 'DebugShape pool[BOX3D_USER_SHAPE_CAPACITY];', 'GpuSlots pool;\n    int poolCapacity;')
            text = replace(text, '} DebugShape;', '    int poolIndex; // stable index; callbacks retain this allocation\n} DebugShape;')
            text = replace(text, 'static AdapterState s_adapter;', r'''static AdapterState s_adapter;

static DebugShape* GetDebugShape(int index)
{
    assert(index >= 0 && index < s_adapter.poolCapacity);
    return (DebugShape*)gpu_slots_get(&s_adapter.pool, index + 1, sizeof(DebugShape), false);
}

// Only the pointer directory moves. Parent/child callback addresses never move.
// The immutable GPU upload reservation is distinct from retained debug metadata:
// compounds retain all children, even when only a few are visible this frame.
static void GrowDebugShapePool(void)
{
    int old = s_adapter.poolCapacity;
    int increment = BOX3D_USER_SHAPE_CAPACITY;
    if (increment > INT_MAX - 1 - old) gpu_slots_oom();
    int capacity = old + increment;
    for (int i = old; i < capacity; ++i)
    {
        DebugShape* us = (DebugShape*)gpu_slots_get(&s_adapter.pool, i + 1, sizeof(DebugShape), true);
        us->kind = Box3DUS_Free;
        us->poolIndex = i;
        us->nextFree = i + 1 < capacity ? i + 1 : BOX3D_FREELIST_END;
    }
    s_adapter.poolCapacity = capacity;
    s_adapter.firstFree = old;
    fprintf(stderr, "sokol-debug-pool: %d stable slots (was %d)\n", capacity, old);
}
''')
            # Existing callbacks use integer child maps and retained pointers.
            # Replace every array access and the sole pointer-subtraction index.
            import re
            text = re.sub(r'&s_adapter\.pool\[([^]]+)\]', r'GetDebugShape(\1)', text)
            text = re.sub(r's_adapter\.pool\[([^]]+)\]\.', r'GetDebugShape(\1)->', text)
            text = replace(text, '(int)( us - s_adapter.pool )', 'us->poolIndex')
            text = text.replace('i < BOX3D_USER_SHAPE_CAPACITY', 'i < s_adapter.poolCapacity')
            text = text.replace('i + 1 < BOX3D_USER_SHAPE_CAPACITY', 'i + 1 < s_adapter.poolCapacity')
            text = replace(text, 'index < BOX3D_USER_SHAPE_CAPACITY', 'index < s_adapter.poolCapacity')
            text = replace(text, 'void InitAdapter( void )\n{', 'void InitAdapter( void )\n{\n    if (!s_adapter.poolCapacity) GrowDebugShapePool();')
            text = replace(text, 'if ( s_adapter.firstFree == BOX3D_FREELIST_END )\n\t{\n\t\treturn -1;\n\t}', 'if ( s_adapter.firstFree == BOX3D_FREELIST_END )\n\t{\n\t\tGrowDebugShapePool();\n\t}')
            text = text.replace('The fixed pool never relocates', 'Allocated chunks never relocate')
            text += r'''
void sample_renderer_release_adapter(void)
{
    if (!s_adapter.poolCapacity) return;
    ResetAdapterPool();
    gpu_slots_release(&s_adapter.pool);
    s_adapter.poolCapacity = 0;
    s_adapter.firstFree = BOX3D_FREELIST_END;
}
'''
        elif path.name == 'renderer.c':
            text = replace(text, '#define SHAPE_CAPACITY 65536', '#define SHAPE_CAPACITY (sample_renderer_capacity())\nstatic uint64_t sample_uploaded_instances;\nuint64_t sample_renderer_instance_count(void) { return sample_uploaded_instances; }')
            text = replace(text, '\tUploadMeshInstances();', '''    sample_uploaded_instances = (uint64_t)UploadMeshInstances()
        + s_gfx.cubeCount + s_gfx.sphereCount + s_gfx.capsuleCount
        + s_gfx.cubeCountXp + s_gfx.sphereCountXp + s_gfx.capsuleCountXp;''')
            text = replace(text, '\tDestroyMeshRegistry();', '\tsample_renderer_release_adapter();\n\tDestroyMeshRegistry();')
        elif path.name == 'geometry_registry.c':
            text = replace(text, '#define MAX_GEOM_INSTANCES_GLOBAL 65536', '#define MAX_GEOM_INSTANCES_GLOBAL (sample_renderer_capacity())')
        text = '#include "sokol_capacity.h"\n' + text
        out = dst/path.name
        if not out.exists() or out.read_text() != text:
            out.write_text(text)

if __name__ == '__main__':
    generate(*map(Path, sys.argv[1:]))
