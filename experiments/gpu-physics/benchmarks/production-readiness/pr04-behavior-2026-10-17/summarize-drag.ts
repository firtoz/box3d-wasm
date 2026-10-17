import{readFileSync,writeFileSync}from'node:fs';import{resolve}from'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17');const r=JSON.parse(readFileSync(p+'/drag-batch3-handle-correction-receipt.json','utf8'));
const rows=r.rows.filter(r=>r.label.match(/^(ordinary|native)-order[01]-(isolated|ground)$/)).map(row=>{
 const log=readFileSync(p+'/'+row.log,'utf8');
 const metrics=log.split('\n').filter(s=>s.startsWith('PHYSICAL engine=')||s.startsWith('TRACKING_APPLICABILITY engine=')||s.startsWith('TIMING combined_completed'));
 if(!row.accepted||!log.includes('PASS independent physical dragging')||metrics.length!==5)throw Error('Incomplete '+row.label);
 const data=Object.fromEntries(metrics.at(-1)!.split(' ').slice(2).map(s=>{const [k,v]=s.split('=');return[k,Number(v)]}));
 return{label:row.label,log:row.log,accepted:row.accepted,metrics,timingPass:data.SPS>=60&&data.p95_ms<=1000/30&&data.max_ms<=250,originalTrajectoryPass:log.includes('Original trajectory result=PASS'),unconditionalTrackingPass:!log.includes('global_screen=FAIL_preserved')};
});
const checks=r.rows.filter(r=>r.label.includes('-ccd_landing_')||r.label.includes('-convex_gpu_ccd_'));
if(r.status!=='PASS'||rows.length!==8||checks.length!==8||checks.some(r=>!r.accepted))throw Error('Incomplete matrix');
const report={status:'PHYSICAL_DRAG_MATRIX_PASS_NOT_PR04_OR_PERFORMANCE_COMPLETION',physicalSceneCells:8,isolatedSteps:1320,groundSteps:3060,totalCombinedSteps:4*(1320+3060),independentBodyObservations:4*(1320+3060)*5*2,focusedCCDChecks:8,rows,checks:checks.map(r=>({label:r.label,accepted:r.accepted,log:r.log})),limitations:['Original loaded trajectory FAIL remains deferred, not repaired. Unconditional tracking screen still FAILS on supported lowering; actual upward impulses justify only the documented free-servo applicability exception.','Timing is combined CPU+GPU completed-step wall time; some first-use maximum latencies fail the frozen250ms target.','Scene libraries are frozen PR06 production inputs; focused CCD Rust test executables include the artifact-only pure-predicate cull change. No final PR07 or complete29CCD regression claim.']};
writeFileSync(p+'/drag-physical-summary.json',JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify(report,null,2));
