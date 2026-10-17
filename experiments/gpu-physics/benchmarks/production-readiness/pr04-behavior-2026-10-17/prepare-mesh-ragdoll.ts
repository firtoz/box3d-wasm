import {existsSync,readFileSync,writeFileSync,readdirSync} from 'node:fs';
import {resolve,join} from 'node:path';
const r=process.cwd(),g=join(r,'experiments/gpu-physics'),p=join(g,'benchmarks/production-readiness/pr04-behavior-2026-10-17');
const phase='mesh-ragdoll-qualification',file=join(p,phase+'-protocol.json');if(existsSync(file))throw Error('Frozen phase exists');
const old=join(g,'artifacts/pr05-pr06-release-candidate-2026-10-17'),dev=join(g,'artifacts/pr04-contact-policy-candidate-2026-10-17');
const previous=JSON.parse(readFileSync(join(p,'joint-c-remaining-protocol.json'),'utf8'));
const ordinary=previous.jobs.find(j=>j.label==='run-ordinary-separation').environment;
const native=previous.jobs.find(j=>j.label==='run-native-reaction').environment;
const modes=[...['twist','twist-negative','tilted','tilted-negative'].map(mode=>({mode,kind:'ragdoll',steps:120,bodies:1})),{mode:'ragdolls-no-contacts',kind:'ragdoll',steps:60,bodies:112},...['grid','torus'].map(mode=>({mode,kind:'mesh',steps:1,bodies:1,input:join(g,'c_abi/fixtures/frozen-mesh',mode+'.txt'),count:mode==='grid'?45:12})),{mode:'ragdolls',kind:'ragdoll',steps:600,bodies:112}];
const raw=(name:string)=>join(p,'raw',name),jobs:any[]=[],outputs:string[]=[];
const add=(label:string,argv:string[],seconds:number,more={})=>jobs.push({label,argv,seconds,...more});
const caches=['/home/firtoz/.cache/box3d-gpu-physics/pipelines','/home/firtoz/.cache/nvidia'];
add('caches-before',['du','-sb',...caches],10);
add('verify-current-libraries',['bun',join(p,'joint-qualification-tools.ts'),'verify'],15);
for(const name of ['human','determinism','utils']){const obj=raw(name+'.o');outputs.push(obj);add('compile-'+name,['cc','-O2','-DNDEBUG','-ffunction-sections','-fdata-sections','-I',join(r,'box3d/include'),'-I',join(r,'box3d/shared'),'-c',join(r,'box3d/shared',name+'.c'),'-o',obj],15);}
for(const [kind,src] of [['ragdoll','ragdoll_reference.cpp'],['mesh','rain_frozen_contact.cpp']]){const obj=raw(kind+'.o');outputs.push(obj);add('compile-'+kind,['g++','-O2','-std=c++17','-ffunction-sections','-fdata-sections','-I',join(r,'box3d/include'),'-I',join(r,'box3d/shared'),'-I',join(g,'c_abi'),'-c',join(g,'c_abi',src),'-o',obj],15);}
for(const engine of ['cpu','ordinary','native']){
 const archive=engine==='cpu'?[join(old,'cpu/box3d_src-libbox3d.a')]:['-Wl,--whole-archive',join(dev,engine+'.a'),join(old,engine+'-gpu/libgpu_samples_api.a'),'-Wl,--no-whole-archive',join(old,engine+'-gpu/box3d_src-libbox3d.a')];
 for(const kind of ['ragdoll','mesh']){const binary=raw(kind+'-'+engine);outputs.push(binary);add('link-'+kind+'-'+engine,['g++','-Wl,--gc-sections',raw(kind+'.o'),...['human','determinism','utils'].map(n=>raw(n+'.o')),...archive,'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',binary],25);}
 for(const order of engine==='cpu'?['1']:['0','1'])for(const mode of modes){
  const env=engine==='cpu'?{}:{...(engine==='ordinary'?ordinary:native),GPU_PHYSICS_LIVE_CONTACT_ORDER:order};
  if(mode.mode==='grid')env.RAIN_FROZEN_GRID='1';
  add('run-'+engine+'-order'+order+'-'+mode.mode,['bun',join(p,'qualification-fixture-tools.ts'),engine,mode.mode,order],engine==='cpu'?30:mode.steps===600?90:30,{environment:env,requireStrings:['"status":"PASS"']});
 }
}
add('verify-final-libraries',['bun',join(p,'joint-qualification-tools.ts'),'verify'],15);
add('caches-after',['du','-sb',...caches],10);
const inputs=[join(p,'qualification-fixture-tools.ts'),join(p,'joint-qualification-tools.ts'),join(p,'joint-c-qualification-protocol.json'),join(p,'current-policy-libraries.json'),join(p,'prepare-mesh-ragdoll.ts'),join(g,'src/sim.rs'),join(g,'c_abi/ragdoll_reference.cpp'),join(g,'c_abi/rain_frozen_contact.cpp'),join(g,'c_abi/human_joint_audit.h'),join(g,'c_abi/fixtures/frozen-mesh/manifest.json'),...modes.flatMap(m=>m.input?[m.input]:[]),...['human','determinism','utils'].flatMap(n=>['c','h'].map(ext=>join(r,'box3d/shared',n+'.'+ext))),...['ordinary','native'].flatMap(engine=>[join(dev,engine+'.a'),join(old,engine+'-gpu/libgpu_samples_api.a'),join(old,engine+'-gpu/box3d_src-libbox3d.a')]),join(old,'cpu/box3d_src-libbox3d.a')];
const protocol={phase,modes,totalSeconds:1750,maxRawBytes:128*1024**2,inputs:[...new Set(inputs)],jobs,cleanup:outputs,cleanupOnFailure:outputs,progressCriterion:'Original unchanged physical/calculation criteria; CPU8 reference cells first;32 GPU cells across both backends/orders. Stop first failure.0 engine builds/new candidates/Rain campaigns/retries/captures.5 compiles15s/6links25s/CPU30s/GPU rag60090s/otherGPU30s/2verify15s/cache10s/1750s phase.',storage:'1GiB temporary including shared driver cache growth; raw128MiB/retained64MiB/free20GiB. CPU references are compact necessary inputs (<2MiB) for pending GPU comparisons, retained until all consumers finish. Objects/six executables deleted after phase on success/failure. Two current development archives retain prior196MiB exception; no new Rust outputs/cache copies.'};
writeFileSync(file,JSON.stringify(protocol,null,2)+'\n');console.log(JSON.stringify({phase,jobs:jobs.length,inputs:protocol.inputs.length,physicsRuns:40,engineBuilds:0,totalSeconds:1750}));
