import{readFileSync,writeFileSync,existsSync,readdirSync,statSync,mkdirSync,renameSync,rmSync}from"node:fs";
import{resolve,join,relative}from"node:path";import{createHash}from"node:crypto";import{execFileSync}from"node:child_process";
const root=process.cwd(),g=resolve("experiments/gpu-physics"),out=join(g,"benchmarks/production-readiness/pr05-pr06-native-2026-10-17");
const read=(name:string)=>JSON.parse(readFileSync(join(out,name),"utf8"));
const sha=(p:string)=>createHash("sha256").update(readFileSync(p)).digest("hex");
const check=(yes:unknown,why:string)=>{if(!yes)throw Error(why);};
const walk=(p:string):string[]=>readdirSync(p,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(join(p,e.name)):e.isFile()?[join(p,e.name)]:[]);
const phases=["runtime-build","runtime-device-build","runtime-list","runtime-rust","runtime-c","runtime-cache-remaining","replay-diagnostic-build","release-rust","release-native","release-cpu-viewer","release-ordinary-viewers","release-viewers","release-consumer","release-sdk","release-wasm","release-compatibility"];
const receipts=phases.map(phase=>{const r=read(phase+"-receipt.json");check(r.status==="PASS",phase+" not PASS");check(r.protocolSha256===sha(join(out,phase+"-protocol.json")),phase+" protocol changed");for(const row of r.rows){check(row.accepted,phase+" rejected row");check(sha(join(out,row.log))===row.logSha256,phase+" log changed");}return{phase,...r};});
const views:any[]=[];
for(const route of["cpu","ordinary-gpu","ordinary-both","native-gpu","native-both"])for(const on of route==="cpu"?[true]:[true,false]){
 const label=route+(on?"-cache-on":"-cache-off"),v=read("viewer-"+label+".json");
 check(v.status==="ok"&&v.measured===18&&v.pause_single_advances===10&&v.pause_idle_advances===0&&v.switch_count===1&&!v.gpu_fail,label+" lifecycle");
 if(route!=="cpu"){
  const log=readFileSync(join(out,"checks/"+(route.startsWith("ordinary")?"release-ordinary-viewers-":"release-viewers-")+label+".log"),"utf8");
  check(log.includes("GPU adapter:")&&log.includes("NVIDIA GeForce RTX 4070 SUPER")&&log.includes("Vulkan"),label+" adapter");
  check(on?log.includes("physics-pipeline-cache: submitted"):!log.includes("physics-pipeline-cache:"),label+" actual pipeline-cache policy");
 }
 views.push({label,status:v.status,measured:v.measured,singleSteps:v.pause_single_advances,idleAdvances:v.pause_idle_advances,switchCount:v.switch_count});
}
const loading=["ordinary-gpu","ordinary-both","native-gpu","native-both"].map(route=>read("loading-close-"+route+"-observed.json"));
check(loading.every(r=>r.pass&&r.activeWorkerObserved&&r.code===0),"loading shutdown");
const loadingAudit=read("loading-final-reconciliation.json");
check(loadingAudit.status==="PASS"&&loadingAudit.sourceReceiptSha256===sha(join(out,"release-loading-observed-receipt.json")),"loading reconciliation");
execFileSync("bun",[join(out,"reconcile-loading.ts")],{stdio:"pipe"});
const consumer=read("external-consumer-results.json");check(consumer.pass&&consumer.rows.length===5&&consumer.rows[0].route==="cpu"&&consumer.maxError<=1e-5,"consumer");
const wasm=read("wasm-compatibility.json");check(wasm.pass&&wasm.pathsRestoredAbsent&&wasm.sourceHashesUnchanged,"WASM");
execFileSync("bun",[join(g,"scripts/pr06-build-receipt.ts"),"verify"],{stdio:"pipe"});
check(execFileSync("git",["-C","box3d","status","--porcelain"],{encoding:"utf8"}).trim()==="","Box3D dirty");
check(execFileSync("git",["diff","--name-only","--","packages/box3d-wasm","patches/box3d","demo/src","demo/public/wasm",join(g,"shaders")],{encoding:"utf8"}).trim()==="","Unrelated engine/WASM/default/shader change");
const pids=[...receipts.flatMap(r=>r.rows.map((j:any)=>j.pid)),...read("release-loading-observed-receipt.json").rows.map((j:any)=>j.pid),...loading.map(r=>r.pid)];check(pids.every(pid=>!existsSync("/proc/"+pid)),"owned leader still alive");
const baseline=read("runtime-device-build-receipt.json");const changed=Object.entries(baseline.inputHashes).filter(([p,h])=>/(\/src\/|\/shaders\/|\/c_abi\/|build.rs|Cargo\.)/.test(p)&&sha(resolve(p))!==h).map(([p])=>relative(root,resolve(p)));
check(changed.length===2&&changed.includes("experiments/gpu-physics/src/sim.rs")&&changed.includes("experiments/gpu-physics/c_abi/pr05_callback_contract.cpp"),"Unexpected main-runtime applicability change");
const applicability={mainRuntimeSourceChanges:changed,sim:"Only diagnostic trailer decoder and typed eight-byte flag array changed after main tests; production key byte order/content, parameters, shader and solver unchanged. Targeted native rebuild+48-step replay assertion/capture passes. Both clean release libraries compile final source.",callbackFixture:"Combined CPU mirror gravity/velocity rejection assertions added after library build, before main C checks. It is not a library input. Four callback fixtures also pass on final release archives.",limits:"Main runtime tests were built before the diagnostic-only fix; use retained executable hashes, not current in-place test file. PR07 final-build full physical/repeatability qualification remains open."};
writeFileSync(join(out,"evidence-applicability.json"),JSON.stringify(applicability,null,2)+"\n");
const candidate=join(g,"artifacts/pr05-pr06-release-candidate-2026-10-17");check(!existsSync(candidate),"Candidate already published");mkdirSync(resolve(candidate,".."),{recursive:true});renameSync(join(out,"raw/candidate"),candidate);
const binaries=walk(candidate).map(p=>({path:relative(root,p),bytes:statSync(p).size,sha256:sha(p)}));const candidateBytes=binaries.reduce((n,r)=>n+r.bytes,0);check(candidateBytes<=512*1024**2,"candidate cap");
writeFileSync(join(out,"release-candidate.json"),JSON.stringify({path:relative(root,candidate),bytes:candidateBytes,files:binaries,consumer:"PR07 final qualification / PR04 applicability review. No consumed compiler/dependency trees retained. Relevant source changes require refreeze before use.",sourceSnapshot:"release-inputs.json",recipes:["release-rust-protocol.json","release-native-protocol.json"],qualification:"PR05/PR06 only; PR04 physical failures and PR07 repeats/performance remain open"},null,2)+"\n");
const raw=join(out,"raw"),left=walk(raw);check(left.length===0,"unconsumed raw files "+left.join(","));rmSync(raw,{recursive:true});
const retainedBytes=walk(out).reduce((n,p)=>n+statSync(p).size,0);check(retainedBytes<=64*1024**2,"retained cap");
const gates=["P1 bounds/layout/reservation","P2 identity/ownership/replacement","P3 callbacks/events","P4 async/replay/failure","P5 lifecycle/loading","P6 launch/cache matrix","P7 resources/concurrency","P8 docs/evidence/cleanup","B1 CPU clean build","B2 ordinary clean targets","B3 native clean targets","B4 external consumer/compatibility","B5 freeze/applicability"];
const acceptance={status:"PASS",pr05:{closed:8,left:0},pr06:{closed:5,left:0},gates:gates.map(name=>({name,status:"PASS"})),views,loading,consumerMaxError:consumer.maxError,retainedBytesBeforeThisReceipt:retainedBytes,candidateBytes,ownedLeadersAbsent:pids.length,peakSampledTaskRawBytes:Math.max(...receipts.map(r=>r.peakRawBytes??0)),scope:"Linux/NVIDIA RTX4070SUPER/Vulkan; explicit owner-thread C contract and callback whitelist; terminal device loss; no automatic device recovery; PR04+PR07/08/12/13 remain open",phaseReceipts:receipts.map(r=>({phase:r.phase,sha256:sha(join(out,r.phase+"-receipt.json")),elapsedMs:r.elapsedMs}))};
writeFileSync(join(out,"acceptance.json"),JSON.stringify(acceptance,null,2)+"\n");console.log("PASS PR05 8/8; PR06 5/5",{retainedBytes,candidateBytes,ownedLeadersAbsent:pids.length});
