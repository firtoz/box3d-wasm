import assert from 'node:assert/strict';
function once(s:string,a:string,b:string){assert.equal(s.split(a).length-1,1,a.slice(0,90));return s.replace(a,b);}
export function overlays(s:string){
 s=once(s,'\tvoid* ctx;\n} WorldVis;','\tvoid* ctx;\n\tfloat inv_dt;\n} WorldVis;');
 s=once(s,'static WorldVis g_worlds[GPU_SAMPLES_WORLD_CAP];','static void overlay_release_joints(unsigned world);\nstatic WorldVis g_worlds[GPU_SAMPLES_WORLD_CAP];');
 s=once(s,'    gpu_slots_release(slots);\n    memset(w, 0, sizeof(*w));','    gpu_slots_release(slots);\n    overlay_release_joints(world.index1);\n    memset(w, 0, sizeof(*w));');
 s=once(s,'#include "draw_color.h"','#include "draw_color.h"\n#include "debug_overlays.h"');
 s=once(s,'draw == NULL || draw->drawShapes == false || draw->DrawShapeFcn == NULL || world_vis(worldId) == NULL','draw == NULL || world_vis(worldId) == NULL');
 s=once(s,'\tuint64_t t_draw = monotonic_ns();\n\tfor (uint32_t i = 1;','\tuint64_t t_draw = monotonic_ns();\n\tDrawSet bodies = {0}, joints = {0}, contacts = {0};\n\tfor (uint32_t i = 1;');
 s=once(s,'\t\tvoid* userShape = ensure_user_shape(worldId, shapeId, s);','\t\tif (draw->drawBounds && draw->DrawBoundsFcn) draw->DrawBoundsFcn(bounds, b3_colorGold, draw->context);\n\t\tif (draw_set_add(&bodies, (DrawKey){s->body.index1,s->body.world0,s->body.generation}))\n\t\t\toverlay_body(worldId, s->body, draw, &joints, &contacts);\n\t\tif (!draw->drawShapes || !draw->DrawShapeFcn) continue;\n\t\tvoid* userShape = ensure_user_shape(worldId, shapeId, s);');
 s=once(s,'\tg_last_draw_list_ms = duration_ms_from_ns(t_draw, monotonic_ns());','\tfree(bodies.keys); free(joints.keys); free(contacts.keys);\n\tg_last_draw_list_ms = duration_ms_from_ns(t_draw, monotonic_ns());');
 return s;
}
export function shim(s:string){
 const declaration=s.match(/void gpu_samples_on_world_created\(b3WorldId world, const b3WorldDef\* def\) (?:__attribute__\(\(weak\)\))?;/)?.[0];assert(declaration);s=once(s,declaration,'void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def) __attribute__((weak));\nvoid gpu_samples_on_world_step(b3WorldId world, float dt) __attribute__((weak));\nvoid gpu_samples_on_joint_created(b3JointId id, const b3JointDef* def) __attribute__((weak));');
 s=once(s,'\tgpu_b3_world_step(worldId, timeStep, subStepCount);','\tgpu_b3_world_step(worldId, timeStep, subStepCount);\n\tif (gpu_samples_on_world_step) gpu_samples_on_world_step(worldId, timeStep);');
 s=once(s,'\tgpu_b3_joint_set_force_threshold(id, base->forceThreshold);','\tif (gpu_samples_on_joint_created) gpu_samples_on_joint_created(id, base);\n\tgpu_b3_joint_set_force_threshold(id, base->forceThreshold);');
 const a='\tdef.base.internalValue = 1152023;';assert.equal(s.split(a).length-1,9);s=s.replaceAll(a,'\tdef.base.drawScale = b3GetLengthUnitsPerMeter();\n'+a);return s;
}
export function dual(s:string){
 s=once(s,'extern void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def);','extern void gpu_samples_on_world_created(b3WorldId world, const b3WorldDef* def);\nextern void gpu_samples_on_world_step(b3WorldId world, float dt);\nextern void gpu_samples_on_joint_created(b3JointId id, const b3JointDef* def);');
 s=once(s,'\tgpu_b3_world_step(worldId, timeStep, subStepCount);','\tgpu_b3_world_step(worldId, timeStep, subStepCount);\n\tgpu_samples_on_world_step(worldId, timeStep);');
 return once(s,'\tgpu_b3_joint_set_force_threshold(joint, base->forceThreshold);','\tgpu_samples_on_joint_created(joint, base);\n\tgpu_b3_joint_set_force_threshold(joint, base->forceThreshold);');
}
