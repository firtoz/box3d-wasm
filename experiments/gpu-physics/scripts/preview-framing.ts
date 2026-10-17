/** Native-only preview framing; post-process local generated copies. */
import {readFileSync,writeFileSync} from 'node:fs';
import {join} from 'node:path';
import assert from 'node:assert/strict';
function once(text:string,old:string,replacement:string):string{assert.equal(text.split(old).length-1,1,'Preview framing anchor changed: '+old.slice(0,120));return text.replace(old,replacement);}
const header='#include "sokol_preview_bounds.h"\n';

export function mainPreview(text:string):string{
    text=once(text,'#include "sokol_bench_hooks.h"\n','#include "sokol_bench_hooks.h"\n'+header);
    text=once(text,'\ts_context.sample->Step();','\tsample_preview_begin_frame(s_context.sample->m_worldId, b3World_GetCounters(s_context.sample->m_worldId).bodyCount == 0);\n\ts_context.sample->Step();');
    text=once(text,'\ts_context.sample->Render();','\ts_context.sample->Render();\n\tsample_preview_end_frame();');
    const initial=['\tb3AABB previewBounds;','\tif (sample_preview_take_initial_frame(s_context.sample->m_worldId, &previewBounds))','\t{','\t\tint previewWidth = sapp_width();','#if defined(BOTH_SAMPLES)','\t\tif (both_split_enabled()) previewWidth /= 2;','#endif','\t\tfloat previewAspect = sapp_height() > 0 ? (float)previewWidth / (float)sapp_height() : 1.0f;','\t\ts_context.camera.Frame(previewBounds, previewAspect, SAMPLE_PREVIEW_FRAME_PADDING);','\t}','\t++s_frame;'].join('\n');
    return once(text,'\t++s_frame;',initial);
}

export function samplePreview(text:string):string{
    text=once(text,'#include "sokol_bench_hooks.h"\n','#include "sokol_bench_hooks.h"\n'+header);
    text=once(text,'\tb3AABB aabb = b3World_GetBounds( m_worldId );','\tb3AABB aabb = b3World_GetBounds( m_worldId );\n\tbool preview = b3World_GetCounters(m_worldId).bodyCount == 0 && sample_preview_get_bounds(m_worldId, &aabb);');
    text=once(text,'\tm_camera->Frame( aabb, aspect, 0.75f );','\tm_camera->Frame( aabb, aspect, preview ? SAMPLE_PREVIEW_FRAME_PADDING : 0.75f );');
    text=once(text,'\t\t\t\tb3AABB aabb = b3World_GetBounds( context->sample->m_worldId );','\t\t\t\tb3AABB aabb = b3World_GetBounds( context->sample->m_worldId );\n\t\t\t\tbool preview = b3World_GetCounters(context->sample->m_worldId).bodyCount == 0 && sample_preview_get_bounds(context->sample->m_worldId, &aabb);');
    return once(text,'\t\t\t\tcam.Frame( aabb, aspect, 0.75f );','\t\t\t\tcam.Frame( aabb, aspect, preview ? SAMPLE_PREVIEW_FRAME_PADDING : 0.75f );');
}

export function collisionPreview(text:string):string{
    const anchor='\t\tm_sweepB.q2 = { { -0.604184389, -0.424355596, 0.0457959622 }, 0.672894001 };';
    return header+once(text,anchor,anchor+'\n\t\tif (m_context->restart == false) sample_preview_request_initial_frame(m_worldId);');
}

export function rendererPreview(text:string):string{
    for(const[fn,call]of[['AppendCube','sample_preview_include_cube(transform, scale);'],['AppendSphere','sample_preview_include_sphere(transform, radius);'],['AppendCapsule','sample_preview_include_capsule(transform, halfLength, radius);']]){
        const pattern=new RegExp('((?:void '+fn+'\\(|void SplitRaw'+fn+'\\()[\\s\\S]*?\\*count \\+= 1;)');
        assert.equal([...text.matchAll(new RegExp(pattern,'g'))].length,1,'Renderer preview anchor changed: '+fn);
        text=text.replace(pattern,'$1\n\t'+call);
    }
    return header+text;
}

export function overlayPreview(text:string):string{
    text=once(text,'\ts_overlay.lineCount++;','\ts_overlay.lineCount++;\n\tsample_preview_include_relative(a, 0.0f);\n\tsample_preview_include_relative(b, 0.0f);');
    return header+once(text,'\ts_overlay.pointCount++;','\ts_overlay.pointCount++;\n\tsample_preview_include_relative(p, 0.0f);');
}

if(import.meta.main){
    const[kind,src,dst]=process.argv.slice(2);assert(src);
    if(kind==='gfx')for(const[name,transform]of[['renderer.c',rendererPreview],['overlay.c',overlayPreview]] as const){const file=join(src,name);writeFileSync(file,transform(readFileSync(file,'utf8')));}
    else{assert(dst);const transform={main:mainPreview,sample:samplePreview,collision:collisionPreview}[kind];assert(transform);writeFileSync(dst,transform(readFileSync(src,'utf8')));}
}
