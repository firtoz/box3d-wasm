import { spawn, spawnSync } from "node:child_process";
import { mkdirSync, writeFileSync, rmSync } from "node:fs";
import { resolve } from "node:path";

// One attempt, one owned viewer, one WM_DELETE_WINDOW request. No restart.
const out=resolve("experiments/gpu-physics/benchmarks/production-readiness/pr05-pr06-native-2026-10-17");
const route=process.argv[2];if(!["ordinary-gpu","ordinary-both","native-gpu","native-both"].includes(route))throw Error("route");
const exe=out+"/raw/candidate/"+route+"/bin-samples_"+(route.endsWith("both")?"both":"gpu");
const helper=out+"/raw/close-window";
const cwd=out+"/raw/loading-cwd-"+route;mkdirSync(cwd);
const child=spawn(exe,["--sample-name","Bodies/Body Type"],{cwd,env:process.env,detached:true,stdio:["ignore","pipe","pipe"]});
let log="",closed=false,window="",title="",reason="",sentAt=0;const began=Date.now();
const kill=()=>{if(child.pid)try{process.kill(-child.pid,"SIGKILL");}catch{}};
for(const s of ["SIGINT","SIGTERM"]as const)process.once(s,()=>{kill();process.exit(143);});
const consume=(b:Buffer)=>{log+=b;if(log.length>1024**2){reason="output cap";kill();}};
child.stdout.on("data",consume);child.stderr.on("data",consume);
const timer=setInterval(()=>{
  if(Date.now()-began>90000){reason="90s process cap";kill();return;}
  if(sentAt)return;
  if(Date.now()-began>15000){reason="15s loading-window observation cap";kill();return;}
  const windows=spawnSync("xdotool",["search","--onlyvisible","--pid",String(child.pid)],{encoding:"utf8",timeout:500});
  for(const w of (windows.stdout??"").trim().split("\n").filter(Boolean)){
    const t=spawnSync("xdotool",["getwindowname",w],{encoding:"utf8",timeout:500}).stdout?.trim()??"";
    if(!t.startsWith("Preparing simulation:"))continue;
    // This adapter message is emitted by scene creation on the loading worker.
    if(!log.includes("GPU adapter:")||log.includes("gpu-loading: ready"))continue;
    window=w;title=t;
    const result=spawnSync(helper,[w],{timeout:1000});
    if(result.status!==0){reason="WM_DELETE helper failed";kill();return;}
    sentAt=Date.now();console.log("WM_DELETE sent during worker-owned scene loading",route,w,t);return;
  }
},50);
const result=await new Promise<{code:number|null,signal:string|null}>((res,rej)=>{child.once("error",rej);child.once("close",(code,signal)=>{closed=true;res({code,signal});});});
clearInterval(timer);kill();
rmSync(cwd,{recursive:true});
writeFileSync(out+"/checks/loading-close-"+route+".log",log);
const pass=closed&&result.code===0&&sentAt>0&&!reason;
writeFileSync(out+"/loading-close-"+route+".json",JSON.stringify({pass,route,pid:child.pid,window,title,activeWorkerObserved:sentAt>0,elapsedMs:Date.now()-began,shutdownMs:sentAt?Date.now()-sentAt:null,...result,reason},null,2)+"\n");
if(!pass)throw Error("Loading shutdown failed: "+JSON.stringify({result,reason,sentAt}));
console.log("PASS loading-worker shutdown",route);
