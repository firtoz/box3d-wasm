#!/usr/bin/env python3
"""Generate local multi-view renderer sources; leave the Box3D submodule clean."""
import pathlib
import sys
src, dst = map(pathlib.Path, sys.argv[1:])
dst.mkdir(parents=True, exist_ok=True)
for path in src.glob('*.c'):
    if path.name == 'sokol_impl.c':
        continue
    text = path.read_text()
    if path.name == 'debug_adapter.c':
        # CPU handles are validated by the dual-world owner, not by GPU IsValid.
        text += '\nvoid SetComparisonSelectedBody(b3BodyId id) { s_adapter.selectedBodyId = id; s_adapter.selectedShapeId = b3_nullShapeId; }\n'
    if path.name == 'renderer.c':
        begin = text.index('void RenderFrame( ')
        end = text.index('void RenderFrameOffscreen(', begin)
        body = text[begin:end]
        body = body.replace('swapChain->width', 'viewWidth')
        body = body.replace('uint64_t tStart = b3GetTicks();',
            'uint64_t tStart = b3GetTicks();\n\tint viewWidth = split_width > 0 ? split_width : swapChain->width;')
        body = body.replace('SG_LOADACTION_DONTCARE', '(split_x == 0 ? SG_LOADACTION_CLEAR : SG_LOADACTION_LOAD)')
        anchor = 'sg_begin_pass( &tmPass );'
        assert body.count(anchor) == 1
        body = body.replace(anchor, anchor + '\n\tsg_apply_viewport(split_x, 0, viewWidth, swapChain->height, true);\n\tsg_apply_scissor_rect(split_x, 0, viewWidth, swapChain->height, true);')
        text = text[:begin] + 'static int split_x, split_width;\nvoid split_render_viewport(int x, int width) { split_x=x; split_width=width; }\n' + body + text[end:]
    text = '#define SPLIT_STREAM_REMAP\n#include "sokol_split_stream.h"\n' + text
    (dst / path.name).write_text(text)
