import { readFileSync, writeFileSync, existsSync, appendFileSync } from 'node:fs';
import { resolve, join } from 'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17');
const dest=join(p,'rain-batch2-profile-protocol.json');
if(existsSync(dest)) throw Error('Frozen phase already exists');
const old=JSON.parse(readFileSync(join(p,'rain-batch1-headless-protocol.json'),'utf8'));
let source=readFileSync(join(p,'rain-headless.cpp'),'utf8');
source=source.replace('using Clock=',`struct Profile { uint64_t physics_step,timestamp_step; uint32_t valid,measured_fields; float values[23]; };
extern "C" void gpu_b3_world_native_profile(b3WorldId,Profile*) __attribute__((weak));
extern "C" float gpu_b3_world_last_encode_ms(b3WorldId) __attribute__((weak));
extern "C" float gpu_b3_world_last_fetch_ms(b3WorldId) __attribute__((weak));
using Clock=`);
source=source.replace('frame<600','frame<336');
source=source.replace('b3World_Step(world,1.0f/60,4);if(gpu_b3_world_wait)', 'b3World_Step(world,1.0f/60,4);auto submitted=Clock::now();if(gpu_b3_world_wait)');
source=source.replace('  std::printf("{\\"kind\\":\\"step\\"',`  if(gpu_b3_world_native_profile){
   Profile profile{};gpu_b3_world_native_profile(world,&profile);assert(profile.valid==1);
   auto val=[&](int field){return (profile.measured_fields&(1u<<field))?profile.values[field]:-1.0f;};
   std::printf("{\\"kind\\":\\"profile\\",\\"step\\":%d,\\"submit_call_ms\\":%.9g,\\"completion_call_ms\\":%.9g,\\"encode_ms\\":%.9g,\\"fetch_ms\\":%.9g,\\"timestamp_step\\":%llu,\\"device_ms\\":%.9g,\\"broadphase_ms\\":%.9g,\\"collide_ms\\":%.9g,\\"solver_ms\\":%.9g}\\n",frame+1,ms(c,submitted),ms(submitted,end),gpu_b3_world_last_encode_ms(world),gpu_b3_world_last_fetch_ms(world),(unsigned long long)profile.timestamp_step,val(0),val(1),val(2),val(3));
  }
  std::printf("{\\"kind\\":\\"step\\"`);
source=source.replace('PASS headless original Rain600 completed','PASS profiled original Rain336 completed');
if(!source.includes('completion_call_ms')||source.includes('frame<600'))throw Error('Source transformation failed');
writeFileSync(join(p,'rain-profile.cpp'),source);
const jobs=old.jobs.filter((j:any)=>!j.label.endsWith('engine-only')).map((j:any)=>{
 j=JSON.parse(JSON.stringify(j));
 j.argv=j.argv.map((s:string)=>s.replaceAll('rain-headless','rain-profile'));
 if(j.requireStrings)j.requireStrings=['PASS profiled original Rain336 completed'];
 return j;
});
const cleanup=old.cleanup.map((s:string)=>s.replaceAll('rain-headless','rain-profile'));
const protocol={...old,purpose:'Batch2 H1 encode/submission cost vs completion/mirror cost; H2 GPU kernel time vs host postprocessing. Existing diagnostics, no policy changes. Original upstream Rain336 includes full occupancy and first recycling. Worker1,dt1/60,4substeps,.05recycle. Timing only; no physical acceptance.',batch:2,startedUnixMs:Date.now(),totalSeconds:400,inputs:old.inputs.map((s:string)=>s.replaceAll('rain-headless.cpp','rain-profile.cpp')),jobs,cleanup,cleanupOnFailure:cleanup,
progressCriterion:'Attribute at least50% loaded step wall cost to submit/completion and compare existing timestamped GPU time. If insufficient, switch profiling approach rather than repeat the cell. No solver candidate, no new timing policy,0enginebuilds,4C+1CPP15s/3links25s/CPU30s+2GPU90s/cache10s/400sphase. Unlaunched cases explicitly retained.',
storage:'Raw128MiB plus shared cache growth within1GiB temporary,retained64MiB/free20GiB. Objects/exes consumed by3runs then deleted on success/failure. stdout capped16MiB,owned groups reaped; shared caches preserved.'};
writeFileSync(dest,JSON.stringify(protocol,null,2)+'\n');
appendFileSync(join(p,'log.md'),'\nContinuation after publication: previous turn was PROGRESS (committed/pushed checkpoint6cd6956a7 and integrity/cleanup evidence). No production edits. Rain/performance batch2 FROZEN in rain-batch2-profile-protocol.json: existing completion/encode/fetch/native timestamp diagnostics on original336step headless Rain through first recycling; CPUfirst/ordinary/native,0enginebuilds/candidates/retries/captures,4C+1CPP15s/3links25s/CPU30s+2GPU90s/cache10s/400sphase,1GiBtemporary/64MiBretained/free20GiB. H1submit vscompletion; H2GPUwork vshostpostprocessing. Attribution>=50% required as progress. Budget now2/3allocated,1left;0/6gatesclosed. All prior FAILED phases remain unchanged.\n');
console.log('Frozen',dest);
