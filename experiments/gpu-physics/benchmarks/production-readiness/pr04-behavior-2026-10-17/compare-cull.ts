import { readFileSync, writeFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { createHash } from 'node:crypto';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17');
const parse=(phase:string,backend:string)=>{
 const path=`${p}/checks/${phase}-run-${backend}-viewer-setters.log`,bytes=readFileSync(path);
 const records=bytes.toString().split('\n').filter(s=>s.startsWith('{"kind":')).map(s=>JSON.parse(s));
 const steps=records.filter(r=>r.kind==='step'),profiles=records.filter(r=>r.kind==='profile');
 if(steps.length!==336||steps.some((r,i)=>r.step!==i+1||r.completed!==i+1)||profiles.length!==336||profiles.some((r,i)=>r.step!==i+1||r.timestamp_step!==i+1))throw Error('Incomplete or stale timing '+path);
 const end=records.find(r=>r.kind==='endpoint');if(end?.humans!==300||end.dynamic_bodies!==4200)throw Error('Incomplete endpoint');
 const tail=steps.slice(-20),v=tail.map(r=>r.physics_ms).sort((a,b)=>a-b),meanMs=v.reduce((n,x)=>n+x,0)/v.length;
 const clocks=Object.fromEntries(['submit_call_ms','completion_call_ms','encode_ms','fetch_ms','device_ms','broadphase_ms','collide_ms','solver_ms'].map(k=>[k,profiles.slice(-20).reduce((n,r)=>n+r[k],0)/20]));
 return{path,sha256:createHash('sha256').update(bytes).digest('hex'),steps:steps.length,endpoint:end.pose_velocity_digest,tail20:{meanMs,SPS:1000/meanMs,simulatedSecondsPerSecond:1000/meanMs/60,p95Ms:v[18],maxMs:v[19]},clocks};
};
const rows=['ordinary','native'].map(backend=>{
 const before=parse('rain-batch2-profile',backend),after=parse('rain-batch3-cull',backend);
 const throughputRatio=after.tail20.SPS/before.tail20.SPS;
 return{backend,before,after,throughputRatio,meets20PercentGain:throughputRatio>=1.2,endpointExact:before.endpoint===after.endpoint};
});
const receipt=JSON.parse(readFileSync(p+'/rain-batch3-cull-receipt.json','utf8'));
const restoration=JSON.parse(readFileSync(p+'/rain-batch3-cull-restoration.json','utf8'));
const checks=receipt.rows.filter(r=>r.label.includes('joint_filter'));
const accepted=receipt.status==='PASS'&&restoration.sourceRestored&&checks.length===4&&checks.every(r=>r.accepted)&&rows.every(r=>r.meets20PercentGain&&r.endpointExact);
const report={status:accepted?'FOCUSED_CANDIDATE_PASS_NOT_PR04_ACCEPTANCE':'CANDIDATE_REJECTED_OR_INCOMPLETE',rows,focusedChecks:checks.map(r=>({label:r.label,accepted:r.accepted,log:r.log})),sourceRestored:restoration.sourceRestored,limitations:'336step development profile and endpoint digest, not full600 joint/support/energy qualification or full semantic repeatability. Frozen60SPS target still applies; a20% development improvement does not satisfy that target.'};
writeFileSync(p+'/rain-batch3-cull-comparison.json',JSON.stringify(report,null,2)+'\n');
console.log(JSON.stringify(report,null,2));
