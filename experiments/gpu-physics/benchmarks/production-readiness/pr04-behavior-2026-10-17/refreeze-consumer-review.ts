import { readFileSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";
const out=resolve("experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17");
const rows=["cpu","ordinary-gpu","ordinary-both","native-gpu","native-both"].map(route=>{
 const log=readFileSync(out+"/checks/refreeze-consumer-run-"+route+".log","utf8");
 const line=log.split("\n").find(l=>l.startsWith("{\"position\""));if(!line)throw Error("No consumer output "+route);
 return {route,...JSON.parse(line)};
});
const cpu=rows[0];let maxError=0;
for(const r of rows){
 for(const key of ["position","velocity","mass","ray_fraction"]){
  const a=Array.isArray(r[key])?r[key]:[r[key]],b=Array.isArray(cpu[key])?cpu[key]:[cpu[key]];
  for(let i=0;i<a.length;i++){const err=Math.abs(a[i]-b[i]);if(!Number.isFinite(err)||err>1e-5)throw Error("Consumer CPU mismatch "+r.route+" "+key+" "+err);maxError=Math.max(maxError,err);}
 }
 if(!r.destroyed)throw Error("teardown");
}
writeFileSync(out+"/refreeze-external-consumer-results.json",JSON.stringify({pass:true,absoluteTolerance:1e-5,maxError,rows},null,2)+"\n");
console.log("PASS five external consumers; actual Box3D CPU first; original1e-5; max error",maxError);
