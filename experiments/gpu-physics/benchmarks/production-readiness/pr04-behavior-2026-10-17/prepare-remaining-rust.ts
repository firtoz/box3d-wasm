import{readFileSync,writeFileSync,existsSync,mkdirSync}from'node:fs';import{resolve,join}from'node:path';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),phase='remaining-rust-qualification',file=p+'/'+phase+'-protocol.json';if(existsSync(file))throw Error('Frozen phase exists');
const cf=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-reconciliation-2026-10-02/qualification-contract.json'),contract=JSON.parse(readFileSync(cf,'utf8'));
const original=JSON.parse(readFileSync(p+'/related-physical-checks-protocol.json','utf8')),current=JSON.parse(readFileSync(p+'/contact-policy-batch1-protocol.json','utf8'));
const done=new Set(original.jobs.filter(j=>j.label.startsWith('ordinary-order0-')).flatMap(j=>j.argv.slice(1).filter(s=>!s.startsWith('--'))));
const remaining=contract.required_Rust_selectors.filter(s=>!done.has(s));if(done.size!==65||remaining.length!==36)throw Error('Original coverage mismatch '+done.size+'/'+remaining.length);
const jobs:any[]=[{label:'caches-before',argv:['du','-sb','/home/firtoz/.cache/box3d-gpu-physics/pipelines','/home/firtoz/.cache/nvidia'],seconds:10}],inputs=[cf,p+'/prepare-remaining-rust.ts',p+'/related-physical-checks-protocol.json',p+'/contact-policy-batch1-protocol.json',resolve('experiments/gpu-physics/src/sim.rs')],cleanup=[];
for(const backend of['ordinary','native']){
 const logFile=p+'/checks/contact-policy-batch1-'+backend+'-tests.log',log=readFileSync(logFile,'utf8'),match=log.match(/Executable unittests[^\n]*\(([^)]+)\)/);if(!match)throw Error('Compiler binary absent');const binary=resolve(match[1]);inputs.push(binary,logFile);
 for(const order of['0','1']){
  const selectors=remaining.filter(s=>{const a=contract.selector_applicability[s];return(!a?.required_ordering||a.required_ordering.includes(Number(order)))&&(!a?.required_backend||a.required_backend.includes(backend==='native'?'native-cached':backend));});
  const scratch=join(p,'raw','remaining-'+backend+'-'+order);mkdirSync(scratch,{recursive:true});cleanup.push(scratch);
  const env=current.jobs.find(j=>j.label===backend+'-order'+order+'-physical').environment;
  jobs.push({label:backend+'-order'+order+'-remaining',argv:[binary,...selectors,'--exact','--nocapture','--test-threads=1'],seconds:120,testCount:selectors.length,environment:{...env,TMPDIR:scratch},cleanup:[scratch]});
 }
}
jobs.push({label:'caches-after',argv:jobs[0].argv,seconds:10});
writeFileSync(file,JSON.stringify({phase,remainingSelectors:remaining,totalSeconds:520,maxRawBytes:128*1024**2,inputs,jobs,cleanup,cleanupOnFailure:cleanup,progressCriterion:'Only36 original as-yet-unqualified Rust selectors;34/35 ordinary and35/36 native perordering applicability=140checks. Exactly four process groups120s each,cache10s/520phase; no engine builds/candidates/Rainruns/retries/captures. Existing65/config evidence reused, not rerun. Stop failed group; original libtest assertions/counts unchanged.',storage:'Default1GiB temporary including sharedcache growth,raw128MiB/retained64MiB/free20GiB. Original testtemporary trace outputs routed to ownedignored TMPDIR and deleted after sourceasserts on bothoutcomes. No cachecopies/newbinaries; existingcompiler-selectedtests nextconsumer=remainingqualification.'},null,2)+'\n');
console.log(JSON.stringify({phase,remaining:remaining.length,groups:jobs.filter(j=>j.testCount).map(j=>({label:j.label,count:j.testCount}))}));
