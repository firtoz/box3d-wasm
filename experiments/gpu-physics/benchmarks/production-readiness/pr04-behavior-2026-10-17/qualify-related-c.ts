import{readFileSync,writeFileSync}from'node:fs';import{resolve}from'node:path';import{createHash}from'node:crypto';
const p=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr04-behavior-2026-10-17'),g=resolve('experiments/gpu-physics'),cf=resolve('experiments/gpu-physics/benchmarks/production-readiness/pr03-contract-reconciliation-2026-10-02/qualification-contract.json');
const hash=(f:string)=>createHash('sha256').update(readFileSync(f)).digest('hex'),read=(f:string)=>JSON.parse(readFileSync(p+'/'+f,'utf8')),contract=JSON.parse(readFileSync(cf,'utf8'));
function proof(phase:string,label:string,allowMarkerOnly=false){const r=read(phase+'-receipt.json'),row=r.rows.find(r=>r.label===label);if(!row||(!row.accepted&&!(allowMarkerOnly&&row.code===0)))throw Error('Incomplete '+phase+'/'+label);if(hash(p+'/'+row.log)!==row.logSha256)throw Error('Changed evidence');return{phase,label,code:row.code,log:row.log,logSha256:row.logSha256,markerReconciled:allowMarkerOnly||undefined};}
const whole=read('joint-whole-confirmation-receipt.json');if(whole.status!=='PASS'||whole.rows.length!==8)throw Error('Whole native lifecycle confirmation incomplete');
const kind=read('joint-kind-qualification-receipt.json'),metadata=read('joint-kind-final-audit-receipt.json');if(kind.status!=='FAILED'||kind.rows.length!==29||kind.rows.some(r=>!r.accepted)||!kind.error.includes('Output collision')||metadata.status!=='PASS')throw Error('Keep/reconcile final metadata-only failure');
const cCells:any[]=[];
for(const [recipeIndex,recipe]of contract.C_scene_recipes.entries())for(const backend of['ordinary','native'])for(const order of[0,1]){
 let status='CURRENT_BUILD_PASS',evidence:any[]=[];const arg=recipe.argv[0],label=backend+'-order'+order;
 if(recipe.fixture==='Benchmark/Rain'){status='OPEN_GATE_B';evidence=[{scope:'Full600/currentphysical/recycling acceptance still open; no probe or endpoint digest substitutes for it.'}];}
 else if(recipe.fixture==='ragdoll_reference.cpp'||recipe.fixture==='rain_frozen_contact.cpp'){
  const mode=recipe.fixture==='rain_frozen_contact.cpp'?recipe.kind:arg,record=read('qualification-results/'+label+'-'+mode+'.json');if(record.status!=='PASS')throw Error('Fixture failed');evidence=[proof('mesh-ragdoll-qualification','run-'+label+'-'+mode),{comparison:'qualification-results/'+label+'-'+mode+'.json',comparisonSha256:hash(p+'/qualification-results/'+label+'-'+mode+'.json')}];
 }
 else if(recipe.fixture==='distance-original.cpp')evidence=[proof('distance-qualification','run-'+label)];
 else if(recipe.fixture==='both_joint_reaction_test.cpp'){
  if(backend==='native')evidence=[proof('joint-whole-confirmation',label+'-whole-reaction')];
  else if(order===0)evidence=[proof('joint-kind-qualification',label+'-reaction')];
  else{evidence=[proof('joint-c-qualification','run-ordinary-reaction',true)];const log=readFileSync(p+'/'+evidence[0].log,'utf8');if(!log.includes('joint reaction: 2856 independent CPU/GPU scalar comparisons passed'))throw Error('Reaction marker mismatch');}
 }
 else if(recipe.fixture==='both_joint_separation_test.cpp'){
  if(order===0)evidence=[proof('joint-kind-qualification',label+'-separation')];
  else if(backend==='ordinary')evidence=[proof('joint-c-remaining','run-ordinary-separation')];
  else evidence=[proof('joint-native-separation','run-native-separation')];
 }
 else if(recipe.fixture==='both_drag_test.cpp'){
  const ground=arg==='--ground';
  if(order===1&&ground)evidence=[proof('contact-policy-batch1',label+'-ground')];
  else{status=order===0?'REUSED_APPLICABLE_MODE0_POLICY_UNCHANGED':'REUSED_APPLICABLE_ISOLATED_CONTACT_POLICY';evidence=[proof('drag-batch3-handle-correction',label+'-'+(ground?'ground':'isolated')),{scope:order===0?'Mode0 already selects paired normals before/after the sole production source change. Frozen CAND source unchanged; physical gateC evidence carried.':'Only normal-contact solver selection changed. Original controlledfixture resets all5bodies atheight100 each132step grab/release, separates x by3m, target excursion x.35/y2m, original velocity/positions remain bounded. No groundimpact or interbodycontact is exercised; joint/picking/defaults unchanged. Existingfree-servo/isolatednumerical checks carried, changedpolicy ground fixture separately passes.',physicalScope:'Preserve original isolated numerical limits; loaded trajectory parity remains deferred. Timing misses remainE.'}];}
 }
 else throw Error('Unmapped recipe '+recipe.fixture);
 cCells.push({recipeIndex,fixture:recipe.fixture,mode:recipe.kind??arg??'default',backend,order,status,evidence});
}
const rust=read('rust-qualification-matrix.json'),traceCells=[];for(const recipe of contract.trace_recipes)for(const c of rust.cells.filter(c=>c.selector===recipe.selector)){if(c.status==='NOT_APPLICABLE')continue;if(!['CURRENT_BUILD_PASS','REUSED_APPLICABLE_PREDICATE_EQUIVALENCE'].includes(c.status))throw Error('Trace fixture not qualified');traceCells.push({selector:recipe.selector,steps:recipe.steps,backend:c.backend,order:c.order,status:c.status,evidence:c.evidence,scope:'Original source writes/validates/consumes temporary semantic states during assertions. No fresh five-process full-future-state qualification or archive-copy claim.'});}
const statuses=(cells:any[])=>cells.reduce((n,c)=>(n[c.status]=(n[c.status]??0)+1,n),{}),counts=statuses(cCells);
if(cCells.length!==56||counts.CURRENT_BUILD_PASS!==46||counts.OPEN_GATE_B!==4||traceCells.length!==50)throw Error('Contract coverage mismatch '+JSON.stringify(counts));
const report={status:'RELATED_PR04_D_REGRESSIONS_QUALIFIED_NOT_FULL_PR04',contractSha256:hash(cf),sourcePolicy:hash(g+'/src/sim.rs'),CRecipeCells:56,relatedCRecipeCells:52,Ccounts:counts,cCells,traceCells:50,traceCounts:statuses(traceCells),traces:traceCells,rustCounts:rust.counts,rustRequired:rust.required,sourceApplicability:{soleProductionChange:'sim.rs policy initialization; git diff442689de1..8d18dc8b6 shows no other Rust/WGSL/Cargo change',mode0:'Both source versions select SOLVER_PAIRED_NORMALS whenordering0. Shader implementation and physics parameters unchanged.',mode1:'Changed policy qualified withfresh physical/numerical/joint/mesh/ragdoll/grounddrag cases; isolateddrag unaffected contact path explicitly reused.',preserved:'Bodyidentity/picking/defaults andBox3D/WASM unchanged; oldfailedreceipts neverrewritten.'},limitations:['Rain4cells remainOPEN_GATE_B; Dclosure doesnot waiveRain/throughput/latency/release orPR07/08final campaigns.','Originalnativewhole60sFAILED remains archived. Current full36world process qualifies bothorders with120s cap derived from measuredperkind cost; not a speed/coldstartup pass.','All165scene dispositions retained; onlyFallingRagdolls physicalfixture individually upgraded, not a freshfullcatalogvisual review.','Modes/API source applicability is explicit. 400Rustentries include114historicalpredicate-equivalence reuses; not400newfinal-buildruns.']};
writeFileSync(p+'/related-qualification.json',JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify({status:report.status,Ccounts:counts,traceCounts:report.traceCounts,rustCounts:rust.counts}));
