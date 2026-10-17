// Read-only reconciliation budget: one <=10s host process, zero builds/GPU runs/retries.
import{readFileSync,writeFileSync,existsSync}from"node:fs";import{resolve}from"node:path";import{createHash}from"node:crypto";
const p=resolve("experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17");
const read=(n:string)=>JSON.parse(readFileSync(p+"/"+n,"utf8"));const sha=(f:string)=>createHash("sha256").update(readFileSync(f)).digest("hex");
const r=read("release-loading-observed-receipt.json"),protocol=read("release-loading-observed-protocol.json");
if(r.status!=="FAILED"||!r.error.includes("ENOENT")||!r.error.includes("raw/close-window")||r.rows.length!==5||!r.rows.every((j:any)=>j.accepted&&j.code===0))throw Error("Unexpected terminal state");
if(r.protocolSha256!==sha(p+"/release-loading-observed-protocol.json"))throw Error("Protocol changed");
for(const j of r.rows)if(sha(p+"/"+j.log)!==j.logSha256||existsSync("/proc/"+j.pid))throw Error("Log/process "+j.label);
const helper=p+"/raw/close-window";
if(!protocol.jobs.at(-1).cleanup.includes(helper)||existsSync(helper))throw Error("Expected consumed helper not removed");
for(const[f,h]of Object.entries(r.inputHashes))if(f!==helper&&sha(f)!==h)throw Error("Pinned source changed "+f);
const loading=["ordinary-gpu","ordinary-both","native-gpu","native-both"].map(route=>read("loading-close-"+route+"-observed.json"));
for(const l of loading){
 if(!l.pass||!l.activeWorkerObserved||l.code!==0||l.signal!==null||l.reason||l.identity!=="Actual XCreateWindow return value and getpid from forwarding observer"||existsSync("/proc/"+l.pid))throw Error("Missing actual shutdown "+l.route);
 const log=readFileSync(p+"/checks/loading-close-"+l.route+"-observed.log","utf8");
 if(!log.includes(`PR05WINDOW pid=${l.pid} id=${l.window}`)||!log.includes("GPU adapter:"))throw Error("Window/worker identity");
}
writeFileSync(p+"/loading-final-reconciliation.json",JSON.stringify({status:"PASS",sourceReceiptSha256:sha(p+"/release-loading-observed-receipt.json"),bookkeepingFailureRetained:true,cause:"Driver hashes all pinned inputs after per-job cleanup. Final job intentionally deleted the consumed helper also listed as an input; all five jobs had already exited zero/accepted. No GPU operation failed. Original receipt remains FAILED; this separate audit certifies only its actual successful checks.",pinnedHelperSha256:r.inputHashes[helper],helperInitiallyCompiledBy:"release-loading-close-receipt.json first successful job; reused unchanged for the corrected identity fixture",allOtherPinnedInputsStillMatch:true,allRuntimeLogsMatch:true,allOwnedProcessesAbsent:true,loading,additionalBuilds:0,additionalGpuRuns:0},null,2)+"\n");
console.log("PASS four actual worker-loading shutdowns; consumed-helper final-hash bookkeeping error reconciled without reruns");
