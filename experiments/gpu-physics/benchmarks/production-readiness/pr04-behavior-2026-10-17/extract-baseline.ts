import{readFileSync,writeFileSync,statSync,existsSync}from"node:fs";
import{resolve}from"node:path";import{createHash}from"node:crypto";
const out=resolve("experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17");
const label=process.argv[2];if(!/^(ragdolls|rain)-(cpu|ordinary|native)$/.test(label))throw Error("label");
const path=out+"/raw/"+label+".json",dest=out+"/baseline-"+label+".json";
if(existsSync(dest))throw Error("Existing baseline; no overwrite");if(statSync(path).size>16*1024**2)throw Error("Unexpected benchmark size");
const bytes=readFileSync(path),r=JSON.parse(bytes.toString());
if(r.status!=="ok"||r.measured!==600||r.frames.length!==600||!r.completed_step_mode||r.health_scan||r.pause_script||r.gpu_fail)throw Error("Incomplete or wrong measurement policy "+label);
const frames=r.frames.map((f:any,i:number)=>{
 if(!Number.isFinite(f.physics_ms)||f.physics_ms<=0||f.completed_step!==i+1||f.submitted_step!==i+1||f.body_count<=0)throw Error("Incomplete/nonpositive completed-step row "+i);
 return{step:f.completed_step,bodies:f.body_count,joints:f.joint_count,physics_ms:f.physics_ms,setters_ms:f.setters_ms,profile_ms:f.profile_ms,draw_ms:f.draw_ms,render_ms:f.render_ms,health_ms:f.health_ms,cadence_ms:f.cadence_ms};
});
function summary(a:any[]){const v=a.map(f=>f.physics_ms).sort((x,y)=>x-y),sum=a.reduce((n,f)=>n+f.physics_ms,0);return{steps:a.length,meanCompletedStepMs:sum/a.length,completedStepsPerSecond:1000*a.length/sum,simulatedSecondsPerSecond:1000*a.length/sum/60,p95Ms:v[Math.ceil(v.length*.95)-1],maxMs:v.at(-1),maxBodies:Math.max(...a.map(f=>f.bodies)),maxJoints:Math.max(...a.map(f=>f.joints)),meanHealthMs:a.reduce((n,f)=>n+f.health_ms,0)/a.length};}
const startupPath=out+"/raw/"+label+"-startup.json";
writeFileSync(dest,JSON.stringify({status:"MEASUREMENT_COMPLETE_NOT_PHYSICS_ACCEPTANCE",label,rawSha256:createHash("sha256").update(bytes).digest("hex"),rawBytes:bytes.length,scene:r.sample,mode:r.mode,workerCount:r.worker_count,sleep:r.enable_sleep,dt:1/60,substeps:4,completedStepMode:r.completed_step_mode,healthEnabled:r.health_scan,all:summary(frames),loadedTail120:summary(frames.slice(480)),startup:existsSync(startupPath)?JSON.parse(readFileSync(startupPath,"utf8")):null,frames,limits:"No full health/body/joint scan. Default zero health fields are not physical evidence. One development baseline per cell, not PR08 repeated performance qualification or headline rendering FPS."},null,2)+"\n");
console.log(label,JSON.stringify(summary(frames)),"tail120",JSON.stringify(summary(frames.slice(480))));
