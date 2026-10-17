/** Extend the qualified generator without editing original engines or prior observer sources. */
import{readFileSync,writeFileSync}from'node:fs';import{dirname,join}from'node:path';import assert from'node:assert/strict';
const D=dirname(import.meta.path);
let body=readFileSync(join(D,'body-mutation-observer.inc.cpp'),'utf8');
const props=`extern "C" float cpu_b3Body_GetGravityScale(b3BodyId) __attribute__((weak));\nextern "C" b3MotionLocks cpu_b3Body_GetMotionLocks(b3BodyId) __attribute__((weak));\nextern "C" bool cpu_b3Body_IsBullet(b3BodyId) __attribute__((weak));\nstatic int lock_bits(b3MotionLocks a){return(a.linearX?1:0)|(a.linearY?2:0)|(a.linearZ?4:0)|(a.angularX?8:0)|(a.angularY?16:0)|(a.angularZ?32:0);}\n`;
body=props+body;
body=body.replace('auto p=b.ref.cpu?',`float gravity=b.ref.cpu?cpu_b3Body_GetGravityScale(b.ref.id):b3Body_GetGravityScale(b.ref.id);\n  auto locks=b.ref.cpu?cpu_b3Body_GetMotionLocks(b.ref.id):b3Body_GetMotionLocks(b.ref.id);\n  bool bullet=b.ref.cpu?cpu_b3Body_IsBullet(b.ref.id):b3Body_IsBullet(b.ref.id);\n  auto p=b.ref.cpu?`);
body=body.replace('b3Quat q=b.ref.cpu?',`std::fprintf(f,",\\\"gravity_scale\\\":%.9g,\\\"lock_bits\\\":%d,\\\"is_bullet\\\":%s",gravity,lock_bits(locks),bullet?"true":"false");\n  b3Quat q=b.ref.cpu?`);
body=body.replace('int count=0,dynamic=0,nonzero=0;',`int count=0,dynamic=0,nonzero=0,lock_counts[64]={};float gravity_min=INFINITY,gravity_max=-INFINITY;`);
body=body.replace('++dynamic;auto v=',`++dynamic;float gravity=cpu?cpu_b3Body_GetGravityScale(b.ref.id):b3Body_GetGravityScale(b.ref.id);gravity_min=std::min(gravity_min,gravity);gravity_max=std::max(gravity_max,gravity);++lock_counts[lock_bits(cpu?cpu_b3Body_GetMotionLocks(b.ref.id):b3Body_GetMotionLocks(b.ref.id))];auto v=`);
const point='unsigned n=0;for(auto& b:mutation_bodies)';assert(body.includes(point));
body=body.replace(point,`// Property aggregates cover every dynamic body, not only the64displayexamples.\n  unsigned n=0;for(auto& b:mutation_bodies)`);
body=body.replace('mutation_body_json(f,b);}std::fputs("]}",f);',`mutation_body_json(f,b);}std::fprintf(f,"],\\\"gravity_range\\\":[%.9g,%.9g],\\\"locks_histogram\\\":[",dynamic?gravity_min:0,dynamic?gravity_max:0);bool first_lock=true;for(int i=0;i<64;++i)if(lock_counts[i]){if(!first_lock)std::fputc(',',f);first_lock=false;std::fprintf(f,"[%d,%d]",i,lock_counts[i]);}std::fputs("]}",f);`);
assert(body.includes('gravity_range'));writeFileSync(join(D,'body-properties-base.inc.cpp'),body);
let generator=readFileSync(join(D,'build-joint-control-observer.ts'),'utf8');
generator=generator.replaceAll("'body-mutation-lifetime-build.json'","'joint-control-build.json'").replaceAll("'joint-control-build-budget.json'","'body-properties-build-budget.json'").replaceAll('raw/joint-control-build','raw/body-properties-build').replaceAll('joint-control-observer','body-properties-observer').replaceAll('joint-control-body.inc.cpp','body-properties-body.inc.cpp').replaceAll("join(D,'joint-control-build.json')","join(D,'body-properties-build.json')");
generator=generator.replace("'Weld','Wheel'","'Weld','Wheel','Motor','Filter'").replaceAll('Parallel|Weld|Wheel','Parallel|Weld|Wheel|Motor|Filter');
generator=generator.replace("const extra=['b3DestroyJoint'","setters.push({returnType:'void',name:'b3Joint_SetConstraintTuning',args:'b3JointId jointId, float hertz, float dampingRatio'});\nconst extra=['b3Body_SetGravityScale','b3Body_SetMotionLocks','b3Body_ApplyLinearImpulse','b3Body_ApplyLinearImpulseToCenter','b3Body_SetTargetTransform','b3DestroyJoint'");
generator=generator.replace("'bool b3Joint_IsValid(b3JointId);'","'void b3Joint_GetConstraintTuning(b3JointId,float*,float*);','bool b3Joint_IsValid(b3JointId);'");
generator=generator.replace('joint_refs.size()<128','joint_refs.size()<512');
// Insert the tuning getter within the valid-joint branch, immediately before existing settings JSON.
const anchor='std::fputs(\\\",\\\\\\\"settings\\\\\\\":{\\\",f);';
// Match literal generator source exactly using the shorter stable substring below.
const marker='std::fputs(\",\\\\\"settings\\\\\":{\",f);';
assert(generator.includes(marker),'settings source marker');
generator=generator.replace(marker,`float hz=0,dr=0;if(r.cpu)cpu_b3Joint_GetConstraintTuning(r.id,&hz,&dr);else b3Joint_GetConstraintTuning(r.id,&hz,&dr);std::fprintf(f,\",\\\\\"constraint_tuning\\\\\":[%.9g,%.9g]\",hz,dr);`+marker);
generator=generator.replace("join(D,'body-mutation-observer.inc.cpp')","join(D,'body-properties-base.inc.cpp')");
generator=generator.replace("writeFileSync(src,source);","source+='\\n#include \"body-properties-extra.inc.cpp\"\\n';writeFileSync(src,source);");
// Link from the current qualified pool and replace its observer rather than the already retired predecessor.
generator=generator.replace("x===join(D,'body-mutation-lifetime-observer.cpp')?src:x.startsWith(join(D,'raw/body-mutation-lifetime-build/'))?x.replace(join(D,'raw/body-mutation-lifetime-build/'),folder+'/')", "x===join(D,'joint-control-observer.cpp')?src:x.startsWith(join(D,'raw/joint-control-build/'))?x.replace(join(D,'raw/joint-control-build/'),folder+'/')");
generator=generator.replace("base=JSON.parse(readFileSync(join(D,'body-properties-build.json')", "base=JSON.parse(readFileSync(join(D,'joint-control-build.json')");
writeFileSync(join(D,'build-body-properties-generated.ts'),generator);
await import('./build-body-properties-generated');
