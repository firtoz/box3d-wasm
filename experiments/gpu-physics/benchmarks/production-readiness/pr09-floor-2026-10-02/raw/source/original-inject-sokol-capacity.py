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
            text = replace(text, '#define BOX3D_USER_SHAPE_CAPACITY 65536', '#define BOX3D_USER_SHAPE_CAPACITY (sample_renderer_capacity())')
            text = replace(text, 'DebugShape pool[BOX3D_USER_SHAPE_CAPACITY];', 'DebugShape* pool;')
            text = replace(text, 'void InitAdapter( void )\n{', '''void InitAdapter( void )
{
    if (!s_adapter.pool) {
        size_t n = (size_t)BOX3D_USER_SHAPE_CAPACITY;
        if (n > SIZE_MAX / sizeof(DebugShape)) abort();
        s_adapter.pool = (DebugShape*)calloc(n, sizeof(DebugShape));
        if (!s_adapter.pool) { fputs("Renderer debug-shape allocation failed\\n", stderr); abort(); }
    }''')
            text += '''
void sample_renderer_release_adapter(void)
{
    if (!s_adapter.pool) return;
    ResetAdapterPool();
    free(s_adapter.pool);
    s_adapter.pool = NULL;
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
