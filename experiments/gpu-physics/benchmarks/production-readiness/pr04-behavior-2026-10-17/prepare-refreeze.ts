import{readFileSync,writeFileSync,existsSync}from'node:fs';import{resolve,join}from'node:path';
const r=process.cwd(),g=join(r,'experiments/gpu-physics'),p=join(g,'benchmarks/production-readiness/pr04-behavior-2026-10-17'),old=join(g,'benchmarks/production-readiness/pr05-pr06-native-2026-10-17');
for(const phase of['refreeze-rust','refreeze-native','refreeze-consumer','refreeze-viewers'])if(existsSync(p+'/'+phase+'-protocol.json'))throw Error('Preserve frozenphase '+phase);
const transform=(s:string)=>s.replaceAll(old,p).replaceAll('pr05-pr06-native-2026-10-17','pr04-behavior-2026-10-17');
let helper=readFileSync(g+'/scripts/pr06-build-receipt.ts','utf8');helper=transform(helper);
helper=helper.replace('} else if(mode === "rust") {',`} else if(mode === "carry-cpu") {
  const source=join(g,"artifacts/pr05-pr06-release-candidate-2026-10-17/cpu");
  const target=join(raw,"candidate/cpu");mkdirSync(target,{recursive:true});
  for(const name of ["bin-samples_cpu","box3d_src-libbox3d.a"])copyFileSync(join(source,name),join(target,name));
  const consumer=join(raw,"external-native-consumer");mkdirSync(consumer,{recursive:true});copyFileSync(join(root,"experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17/external-consumer.cpp"),join(consumer,"main.cpp"));
  console.log("Unchanged CPU candidate carried; external consumer input copied");
} else if(mode === "rust") {`);
writeFileSync(p+'/refreeze-build-receipt.ts',helper);
let review=readFileSync(g+'/scripts/pr06-consumer-review.ts','utf8');review=transform(review).replaceAll('release-consumer-run-','refreeze-consumer-run-').replace('external-consumer-results.json','refreeze-external-consumer-results.json');writeFileSync(p+'/refreeze-consumer-review.ts',review);
const inputOld=JSON.parse(readFileSync(old+'/release-inputs.json','utf8'));
const production=Object.keys(inputOld.sourceHashes).filter(k=>/^experiments\/gpu-physics\/(src\/|shaders\/|c_abi\/|native-samples\/|Cargo|build\.rs)/.test(k)).map(k=>resolve(k));
const baseInputs=[p+'/prepare-refreeze.ts',p+'/refreeze-build-receipt.ts',...production,g+'/target/native-backend/cargo-config.toml',g+'/target/native-workspace/experiments/gpu-physics/Cargo.toml',g+'/target/native-workspace/experiments/gpu-physics/Cargo.lock'];
const raw=(s:string)=>p+'/raw/'+s,cacheJob=(label:string)=>({label,argv:['du','-sb','/home/firtoz/.cache/box3d-gpu-physics/pipelines','/home/firtoz/.cache/nvidia'],seconds:10});
const write=(phase:string,protocol:any)=>{if(new Set(protocol.jobs.map(j=>j.label)).size!==protocol.jobs.length)throw Error('Duplicate labels');writeFileSync(p+'/'+phase+'-protocol.json',JSON.stringify({...protocol,inputs:[...new Set(protocol.inputs)]},null,2)+'\n');};
const originalRust=JSON.parse(readFileSync(old+'/release-rust-protocol.json','utf8'));const rust=JSON.parse(transform(JSON.stringify(originalRust)));
for(const job of rust.jobs)job.argv=job.argv.map(a=>a===g+'/scripts/pr06-build-receipt.ts'?p+'/refreeze-build-receipt.ts':a);
for(const backend of['ordinary','native'])rust.jobs.find(j=>j.label===backend+'-receipt').cleanup=[raw('rust-'+backend)];
rust.jobs.push({label:'verify-source-inputs',argv:['bun',p+'/refreeze-build-receipt.ts','verify'],seconds:30});
Object.assign(rust,{totalSeconds:1350,inputs:baseInputs,cleanup:[],cleanupOnFailure:[raw('rust-ordinary'),raw('rust-native'),raw('candidate')],maxRawBytes:6*1024**3,purpose:'F release qualification, not new Rain/performance experiment. Two initiallyempty offline/locked Rust targets, same original releasefeatures andj2, currentretainedsource.600s each; source/dependency receipts/copy/cleanup before nextstage. No production edits/experiment feature/cachecopies/retries.',storage:'Explicit cleanbuild exception6GiBtemporary inclsharedcachegrowth/512MiBnewignoredreleasecandidate retainedforcurrentconsumer andfuturePR07/refreeze;64MiBdiagnostics/free20GiB. Buildtreesdeletedafterlibrary receipts, candidate remains nextconsumer=4nativebuilds/externalconsumers/viewers.'});
write('refreeze-rust',rust);
const originalNative=JSON.parse(readFileSync(old+'/release-native-protocol.json','utf8'));const native=JSON.parse(transform(JSON.stringify(originalNative)));
native.jobs=native.jobs.filter(j=>!j.label.startsWith('cpu-'));
for(const job of native.jobs){job.argv=job.argv.map(a=>a===g+'/scripts/pr06-build-receipt.ts'?p+'/refreeze-build-receipt.ts':a);if(job.label.endsWith('-configure')){const route=job.label.replace('-configure','');job.argv.push('-DFETCHCONTENT_BASE_DIR='+raw('cmake-'+route+'/_deps'),...['imgui','implot','nfd'].map(n=>'-DFETCHCONTENT_SOURCE_DIR_'+n.toUpperCase()+'='+r+'/box3d/.fetchcontent-cache/'+n+'-src'));}}
native.jobs.unshift({label:'carry-unchanged-cpu',argv:['bun',p+'/refreeze-build-receipt.ts','carry-cpu'],seconds:15});
const candidateLibs=['ordinary','native'].map(n=>raw('candidate/'+n+'/libgpu_physics.a'));
Object.assign(native,{inputs:[...baseInputs,p+'/release-inputs.json',...candidateLibs],totalSeconds:1400,maxRawBytes:6*1024**3,cleanup:[],cleanupOnFailure:['ordinary-gpu','ordinary-both','native-gpu','native-both'].map(n=>raw('cmake-'+n)),purpose:'Four independent empty nativebuilds withfresh objects/dependencybuilds; reuseonlypinneddependency sources. CPU unchanged/source-audited carriedfromoldclean candidate. No Rustsource/Box3D/WASM edit. Configure45/build240/receipt30 each,1400phase. No retries/captures.',storage:rust.storage});write('refreeze-native',native);
const consumer=JSON.parse(transform(readFileSync(old+'/release-consumer-protocol.json','utf8')));
for(const job of consumer.jobs)job.argv=job.argv.map(a=>a===g+'/scripts/pr06-consumer-review.ts'?p+'/refreeze-consumer-review.ts':a);
consumer.jobs.unshift(cacheJob('caches-before'));consumer.jobs.push(cacheJob('caches-after'));
Object.assign(consumer,{inputs:[p+'/refreeze-consumer-review.ts',old+'/external-consumer.cpp',g+'/c_abi/pr05_callback_contract.cpp',...candidateLibs],totalSeconds:950,purpose:'OriginalCPUfirst5externalconsumers at1e-5 and4callbackcontrols onnewclean libraries; samepublicAPI/defaults/setup. Not a solver/performance experiment.',maxRawBytes:6*1024**3,storage:rust.storage});
consumer.cleanupOnFailure=[raw('external-native-consumer')];write('refreeze-consumer',consumer);
const viewers:any[]=[];for(const file of['release-cpu-viewer-protocol.json','release-ordinary-viewers-protocol.json','release-viewers-protocol.json']){const c=JSON.parse(transform(readFileSync(old+'/'+file,'utf8')));viewers.push(...c.jobs);}
for(const job of viewers){if(job.cwd)job.preparedCwd=true;}
write('refreeze-viewers',{totalSeconds:1150,maxRawBytes:6*1024**3,inputs:[p+'/prepare-refreeze.ts',...candidateLibs],jobs:[cacheJob('caches-before'),...viewers,cacheJob('caches-after')],cleanup:[],cleanupOnFailure:viewers.filter(j=>j.cwd).map(j=>j.cwd),purpose:'Original9CPUfirst/cacheon-off/pause/singlestep/sceneswitch viewer cells onnewclean nativeexecutables,120s each. No catalog sweep/captures/performance claim. SameoriginalCLIcontroller andphysicaldefaults.',storage:rust.storage});
console.log('Frozen F phases:2Rust/4nativecleanbuilds,original5consumers+4callbacks,9viewercontrols. No Raininvestigationrenewal.');
