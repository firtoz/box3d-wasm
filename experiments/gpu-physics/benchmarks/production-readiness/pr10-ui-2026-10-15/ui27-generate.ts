import{D,read,sha}from'./review-closeout';import{collisionControls,meshScalePreview}from'../../../scripts/inject-sample-controls';import{collisionPreview}from'../../../scripts/preview-framing';import{readFileSync,writeFileSync}from'node:fs';import assert from'node:assert/strict';import{join}from'node:path';
const b=read('ui27-repair-budget.json'),original=readFileSync('box3d/samples/sample_collision.cpp','utf8'),prior=readFileSync(b.old_collision_object.slice(0,-2),'utf8'),production=collisionPreview(collisionControls(original));assert(production===meshScalePreview(prior),'Only two MeshScale transforms change in qualified source');
const helper=[
'static void ui27_preview_boundary(b3Pos start, b3Vec3 translation, int count, float fraction, b3WorldTransform transform)',
'{',
'\tif (!std::getenv("GPU_PR10_MESH_PREVIEW_BOUNDARY")) return;',
'\tconst char* path=std::getenv("GPU_PR10_API_TRACE");',
'\tif (path) if (FILE* f=std::fopen(path,"a")) {',
'\t\tstd::fprintf(f,"{\\"api\\":\\"ui27_mesh_scale_preview\\",\\"frame\\":%d,\\"start\\":[%.17g,%.17g,%.17g],\\"translation\\":[%.9g,%.9g,%.9g],\\"callback_count\\":%d,\\"fraction\\":%.9g,\\"transform\\":[%.17g,%.17g,%.17g]}\\n",ImGui::GetFrameCount(),start.x,start.y,start.z,translation.x,translation.y,translation.z,count,fraction,transform.p.x,transform.p.y,transform.p.z);std::fclose(f);',
'\t}',
'}',
].join('\n');
const start=production.indexOf('class MeshScale : public Sample\n'),end=production.indexOf('class ShapeCast : public Sample\n',start);let mesh=production.slice(start,end);const anchor='\t\t\t\tDrawSolidSphere( transform, sphere, MakeColor(';assert.equal(mesh.split(anchor).length-1,2);mesh=mesh.replaceAll(anchor,'\t\t\t\tui27_preview_boundary(m_start, rayTranslation, context.count, context.count > 0 ? context.fractions[0] : 1.0f, transform);\n'+anchor);writeFileSync(join(D,'raw/ui27-build/sample_collision_ui.cpp'),'#include <cstdio>\n#include <cstdlib>\n'+production.slice(0,start)+helper+'\n'+mesh+production.slice(end));console.log('UI27 production delta: two transforms; diagnostic-only native context/transform observation');
