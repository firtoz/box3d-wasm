/** Terminal evidence only. No builds, viewers or replacement runtime checks. */
import {readFileSync,writeFileSync,existsSync,readdirSync,statSync,lstatSync,statfsSync,unlinkSync} from 'node:fs';
import {dirname,join,relative} from 'node:path';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const D=dirname(import.meta.path), R=process.cwd();
const read=(f:string)=>JSON.parse(readFileSync(join(D,f),'utf8'));
const write=(f:string,j:unknown)=>writeFileSync(join(D,f),JSON.stringify(j,null,2)+'\n');
const md=(f:string,s:string)=>writeFileSync(join(D,f),s);
const hash=(p:string)=>createHash('sha256').update(readFileSync(p)).digest('hex');
assert.equal(read('distance-closeout-budget.json').status,'FROZEN');
assert.equal(read('distance-controls-tail-matrix.json').status,'REVIEW_COMPLETE');
assert.equal(read('empty-world-framing-matrix.json').status,'REVIEW_COMPLETE');
const routes=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];
const fixtures=['distance-debug','shape-distance','time-of-impact'];
const fields=['real_cpu','ordinary_gpu','native_gpu','ordinary_combined','native_combined'];
const originalFolder='raw/distance-controls-runs/distance-debug/cpu';
const original=read(originalFolder+'/receipt.json');
assert.equal(original.status,'FAILURE_RETAINED');
const selected:any[]=[], framed:any[]=[], all:any[]=[{folder:originalFolder,r:original}];
let checks=Object.keys(original.checks).length, findings=0, crashes=0;
for(const route of routes)for(const fixture of fixtures){
  const folder=`raw/${route==='cpu'&&fixture!=='distance-debug'?'distance-controls-runs':'distance-controls-tail-runs'}/${fixture}/${route}`;
  const r=read(folder+'/receipt.json');
  assert(r.valid_checks_complete); assert.equal(r.core_limit_bytes,0); assert(r.cache_growth_within_budget);
  assert.equal(r.binary_sha256,read('distance-control-build.json').viewers.find((v:any)=>v.label===route).sha256);
  if(fixture==='distance-debug'){assert.equal(r.status,'PASS_CONTROLS');assert.equal(r.findings.length,0);assert.equal(r.app_terminal_exit,0);}
  else {assert.equal(r.findings.length,fixture==='shape-distance'?3:5);assert(r.findings.every((f:any)=>/^UI1[567]\//.test(f.check)));}
  if(fixture==='time-of-impact'){
    assert.equal(r.app_terminal_signal,'SIGSEGV');assert.equal(r.negative_crash.invalid.input.A.count,0);crashes++;
  }else if(fixture==='shape-distance'){
    const bad=route==='cpu'?r.cache_transition.invalid[0]:r.negative_crash.invalid;
    assert.equal(bad.input.A.count,1);assert.equal(bad.cache_before.a[0],1);assert.equal(bad.cache_before.count,2);
    if(route==='cpu'){assert.equal(r.app_terminal_exit,0);assert.equal(r.app_terminal_signal,null);}
    else{assert.equal(r.app_terminal_signal,'SIGILL');crashes++;}
    for(const f of r.policy_findings.filter((f:any)=>f.issue==='UI16')){
      assert(Math.abs(f.context.hidden.distance.output.distance-.45)<1e-5);
    }
    assert.equal(r.mouse_cases.length,3);
  }
  for(const s of Object.values(r.screenshots) as any[])assert.equal(hash(s.path),s.sha256);
  checks+=Object.keys(r.checks).length;findings+=r.findings.length;
  selected.push({route,fixture,receipt:folder+'/receipt.json',status:r.status,checks:r.checks,widget_cases:r.widget_cases,mouse_cases:r.mouse_cases,policy_findings:r.policy_findings,negative_crash:r.negative_crash,cache_transition:r.cache_transition,screenshots:r.screenshots,elapsed_seconds:r.elapsed_seconds,app_pid:r.app_pid,driver_pid:r.driver_pid,exit:r.app_terminal_exit,signal:r.app_terminal_signal,shared_cache_growth_bytes:r.shared_cache_growth_bytes});
  all.push({folder,r});
}
assert.equal(checks,388);assert.equal(findings,40);assert.equal(crashes,9);
for(const route of routes){
  const folder=`raw/empty-world-framing-runs/${route}`,r=read(folder+'/receipt.json');
  assert.equal(r.app_terminal_exit,0);assert.equal(r.app_terminal_signal,null);assert.equal(r.core_limit_bytes,0);assert(r.cache_growth_within_budget);
  assert.equal(r.checks['actual pan changes camera'],false);
  assert.equal(r.checks['UI18 framing reaches actual preview'],false);
  for(const key of ['initial','keyboard','menu'])assert(r.framing[key].all_capsules_outside);
  assert(r.checks['actual preview poses remainunchanged acrosscamera controls']);
  for(const s of Object.values(r.screenshots) as any[])assert.equal(hash(s.path),s.sha256);
  framed.push({route,receipt:folder+'/receipt.json',checks:r.checks,framing:r.framing,screenshots:r.screenshots,elapsed_seconds:r.elapsed_seconds,app_pid:r.app_pid,driver_pid:r.driver_pid,shared_cache_growth_bytes:r.shared_cache_growth_bytes});all.push({folder,r});
}
const pids=[...new Set(all.flatMap(({r})=>[r.app_pid,r.driver_pid]).filter(Number.isInteger))];
for(const pid of pids)assert(!existsSync('/proc/'+pid),`Owned PID still present: ${pid}`);
const coredir='/var/lib/systemd/coredump';
const cores=existsSync(coredir)?readdirSync(coredir).filter(n=>all.some(({r})=>r.app_pid&&n.includes('.'+r.app_pid+'.'))):[];
assert.equal(cores.length,0,'Unexpected owned core files');
const build=read('distance-control-build.json');assert.equal(build.status,'PASS');
for(const [path,sha] of Object.entries(build.input_pins))assert.equal(hash(path),sha,`Original input changed: ${path}`);
for(const v of build.viewers)assert.equal(hash(v.binary),v.sha256);
write('distance-controls-acceptance.json',{status:'REVIEWED_THREE_TOOLS_WITH_UI15_UI16_UI17_UNFIXED',language:'TypeScript/Bun',routes_CPU_first:routes,widget_calls:11,custom_input_sample_groups:1,recorded_check_evaluations:checks,true_checks:checks-findings,policy_findings:findings,deliberate_negative_faults:crashes,selected,original_unfinished_distance_debug:originalFolder+'/receipt.json',harness_resolution:'Original Ctrl-click hit the slider label rather than its 96px bar. Only the unfinished Distance Debug CPU fixture ran again; successful Shape Distance/TOI CPU cases were preserved. Driver now targets rect.minX+12. Original failed receipt unchanged.',limits:['These zero-body tools use actual native C calculation/draw operations on all routes; not GPU physics qualification.','Negative cache/enum faults are confirmed findings, never feature passes or clean quits.','TOI A fourth choice faulted on all five routes. B fourth choice shares MakeProxy but was not separately faulted.'],input_pins_unchanged:Object.keys(build.input_pins).length,owned_pids_absent:pids,owned_core_files:cores});
write('empty-world-framing-acceptance.json',{status:'UI18_CONFIRMED_ALL_FIVE_ROUTES_UNFIXED',routes_CPU_first:routes,framed,original_checks:30,confirmed_framing_failures:5,rejected_harness_pan_assertions:5,harness_resolution:'Attempted middle-button pan omitted Alt. Camera::OnEvent requires mods==ALT and middleDown. The five false pan predicates are invalid stimulus assumptions, not product defects. Unchanged receipts retained; initial/F/menu framing independently confirms UI18.',cause:'TimeOfImpact creates direct preview geometry but zero physics bodies. It inherits FocusBounds=false; F falls through to FocusHome/world bounds, and menu Frame Camera also uses world bounds. Neither frames the actual preview. GPU fallback world bounds and CPU empty bounds explain different camera responses.',limits:['Frustum analysis uses actual UI camera rounded to 0.1 and actual draw poses, proxy-derived bounding sphere radius about .22, FOV50 from main.cpp. All outside margins are substantial; no exact pixel assertion.','UI18 runtime confirmation is Time of Impact only. Other zero-body tools share the potential world-bounds path but are not runtime framing-qualified.','The constructor camera default and original geometry/sweeps were preserved.'],owned_pids_absent:pids,owned_core_files:cores});
const issueData=[
 {id:'UI15',defect:'Time of Impact menus expose four labels for three shape enums; fourth choice builds a zero-vertex proxy and crashes',cause:'Menu point/segment/triangle/box is inconsistent with enum box/capsule/triangle and MakeProxy switch.',affected:'Both menus mislabel box and capsule on all five routes. Selecting A index3 captures count0 then SIGSEGV all five. B index3 is source-applicable through the same helper, not separately faulted.',next:'After review, correct generated native menu labels/options, preserving the three supported shapes/defaults and Box3D.'},
 {id:'UI16',defect:'Shape Distance retains hidden radii after switching to triangle/box',cause:'MakeProxy copies radius after the type change; the UI hides radius and DrawShape omits rounded triangle/box geometry.',affected:'All five routes: A triangle radius.2/B box radius.1 still enter GJK and produce distance.45 versus .75 with radii0; outlines omit radii.',next:'After review, reconcile native preview radius policy and displayed geometry on type transitions; preserve defaults and engine calculations.'},
 {id:'UI17',defect:'Shape Distance reuses support indices outside a changed proxy vertex set',cause:'Shape combo rebuilds the proxy without invalidating its simplex cache before b3ShapeDistance reads cached indices.',affected:'All five routes: cached A indices[1,0] are reused when A count becomes1. CPU records invalid input and survives; all four GPU viewers fault SIGILL at the unchanged native operation. This is caller input invalidation, not GPU solver diagnosis.',next:'After review, invalidate the native tool cache on proxy vertex changes, with transition regression checks.'},
 {id:'UI18',defect:'Time of Impact moving preview is offscreen and neither F nor Frame Camera includes it',cause:'Direct preview geometry has no physics bodies; inherited focus and menu framing use physics-world bounds rather than preview bounds.',affected:'Confirmed Time of Impact on actual CPU, ordinary/native GPU and both combined. Default/F/menu leave all three moving capsule preview poses fully outside. CPU radius10 stays unchanged; GPU2.8 and combined F3.1/menu2.8 frame around the wrong bounds. Other zero-body tools are source-applicable but runtime framing-unreviewed.',next:'After review, frame actual preview bounds in the native host while preserving geometry, physics defaults and Box3D.'}
];
const issues=read('issues.json');for(const issue of issueData){const entry={...issue,status:'CONFIRMED_UNFIXED',evidence:[issue.id+'.md',issue.id==='UI18'?'empty-world-framing-acceptance.json':'distance-controls-acceptance.json']};const idx=issues.findIndex((x:any)=>x.id===issue.id);if(idx<0)issues.push(entry);else issues[idx]=entry;
  md(issue.id+'.md',`# ${issue.id} — ${issue.defect}\n\n- [ ] Confirmed, unfixed; review must finish before repair.\n\n${issue.affected}\n\nCause: ${issue.cause}\n\n[Actual boundary evidence](${entry.evidence[1]}), [CPU-first interaction grid](distance-controls-grid.html). These are review states, not repair before/after.\n\n${issue.next}\n`);
}write('issues.json',issues);
const inv=read('inventory.json'),app=read('sample-applicability.json');
const names={'DistanceDebug':'distance-debug','ShapeDistance':'shape-distance','TimeOfImpact':'time-of-impact'};
for(const s of inv.native_samples.filter((s:any)=>s.class in names)){
  const fixture=names[s.class as keyof typeof names],ids=s.class==='ShapeDistance'?['UI16','UI17']:s.class==='TimeOfImpact'?['UI15','UI18']:[];
  for(const w of s.widgets){w.status=ids.length?'REVIEWED_WITH_CONFIRMED_DEFECTS':'REVIEWED_ACTUAL_INPUT_AND_DRAW_EFFECT';w.evidence='distance-controls-acceptance.json';w.issues=ids;for(const field of fields)w[field]=ids.length?'REVIEWED_ACTUAL_INPUT_EFFECT_AND_RECORDED_DEFECTS':'CHECKED_ACTUAL_INPUT_AND_DRAW_EFFECT';}
  s.checked_routes=routes;s.issues=[...new Set([...s.issues,...ids])];s.widget_review={status:'REVIEWED_ACTUAL_NATIVE_CALCULATION_AND_DRAW_EFFECT_WITH_EXPLICIT_DEFECTS',evidence:'distance-controls-acceptance.json',scope:'All5routes, zero-body native preview tool, not GPU solver qualification.'};
  if(s.class==='ShapeDistance')s.custom_input_review={status:'REVIEWED_ACTUAL_TRANSLATE_ROTATE_ALT_IGNORE_AND_RELEASE',evidence:'distance-controls-acceptance.json',routes};
  s.status='REVIEW_COMPLETE_WITH_SHARED_DEFECTS_AND_RECORDED_EXCLUSIONS';s.interaction_review={status:s.status,method:'ACTUAL_SAMPLE_WIDGET_AND_APPLICABLE_CUSTOM_MOUSE_REVIEW_PLUS_CHECKED_SHARED_UI_REUSE',evidence:'sample-applicability.json'};
  const row={id:s.id,sample:s.cpu_identity,method:s.interaction_review.method,inheritance:s.inheritance,source_hashes:Object.fromEntries(s.inheritance.map((c:any)=>[c.source,inv.source_sha256[c.source]])),sample_widget_calls:s.widgets.length,declared_input_methods:s.declared_input_methods,sample_evidence:[{evidence:'distance-controls-acceptance.json',fixture,scope:'Actual controls and native calculation/draw effects all5routes; applicable ShapeDistance gestures; known defects explicit'},...(s.class==='TimeOfImpact'?[{evidence:'empty-world-framing-acceptance.json',scope:'UI18 all5routes, not a feature pass'}]:[])],status:s.status,not_new_runtime_execution:false};
  const idx=app.rows.findIndex((r:any)=>r.id===s.id);if(idx<0)app.rows.push(row);else app.rows[idx]=row;
}
const reviewed=inv.native_samples.flatMap((s:any)=>s.widgets).filter((w:any)=>w.status!=='UNREVIEWED').length;
assert.equal(reviewed,89);assert.equal(app.rows.length,113);
Object.assign(inv.counts,{sample_widget_calls_reviewed:89,sample_widget_calls_remaining:253,interaction_samples_done:113,interaction_samples_left:52,interaction_samples_with_actual_specific_widgets_reviewed:19,custom_input_sample_groups_reviewed:10,custom_input_sample_groups_remaining:13});
Object.assign(app,{status:'RECONCILED_113_SAMPLE_REVIEW_DISPOSITIONS_52_REMAIN',actual_widget_samples:19,limits:'19actualspecificcontrolfixtures+94explicitsharedimplementationreuses;not113newruntimesorfeaturepasses. UI03–UI18open;52rows needremainingcontrols/handlers.'});
write('inventory.json',inv);write('sample-applicability.json',app);
md('sample-applicability.md','# Per-sample interaction review applicability\n\n**113 reviewed /52 remaining:**19 actual sample control fixtures plus94 explicit shared-code reuses. These are review dispositions with known defects, not113 feature passes or new runtimes. [Every source/evidence row](sample-applicability.json). Original PR09 runtime limits remain unchanged.\n\n|ID|Sample|Review method|\n|---|---|---|\n'+app.rows.map((r:any)=>`|${r.id}|${r.sample}|${r.method}|`).join('\n')+'\n');
const esc=(s:string)=>s.replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('"','&quot;');
const html=['<!doctype html><meta charset="utf-8"><title>PR10 collision tools — CPU first</title><style>body{font:16px system-ui;background:#17191d;color:#eee}table{border-collapse:collapse}th,td{padding:8px;border:1px solid #444;vertical-align:top}img{width:400px}</style><h1>Collision tool interaction review</h1><p>Real Box3D CPU first. Default/changed review states, not repaired before/after. UI15–UI18 remain open. Original negative faults and the rejected no-Alt pan assertion are explicit in acceptance.</p><table><tr><th>Tool/state</th>',...routes.map(r=>'<th>'+r+'</th>'),'</tr>'];
for(const [fixture,states] of [['distance-debug',['distance-debug-before','distance-debug-changed']],['shape-distance',['shape-distance-default','shape-distance-hidden-radii','shape-distance-indices','shape-distance-simplex']],['time-of-impact',['toi-default','invalid-toi-option-popup']],['framing',['default-preview','after-frame-controls']]] as [string,string[]][]){
  for(const state of states){html.push('<tr><th>'+fixture+'/'+state+'</th>');for(const route of routes){const row=fixture==='framing'?framed.find(r=>r.route===route):selected.find(r=>r.route===route&&r.fixture===fixture);const p=relative(D,row.screenshots[state].path);html.push(`<td><a href="${esc(p)}"><img src="${esc(p)}" alt="${esc(route+' '+state)}"></a></td>`)}html.push('</tr>');}}
html.push('</table>');md('distance-controls-grid.html',html.join('\n'));
md('distance-controls.md',`# Collision tool review\n\n**11 widget entries and Shape Distance's custom mouse group reviewed on all five routes**, real Box3D CPU first. [Grid](distance-controls-grid.html), [actual GJK/TOI/draw acceptance](distance-controls-acceptance.json), [framing acceptance](empty-world-framing-acceptance.json).\n\nDistance Debug changes/restores actual selected simplex draw geometry while keeping calculation unchanged. Shape Distance exercises both type menus, both radius inputs, indices/world labels, cache, simplex display/index, translation/Shift rotation/Alt-ignore/release and restart defaults. Time of Impact exercises both supported menu choices and unchanged sweeps, then the invalid fourth A option last. These zero-body tools use native C algorithms; GPU solver correctness is outside this batch.\n\nUI15 wrong labels/invalid TOI enum, UI16 hidden active radii, UI17 invalid cached indices and UI18 missing preview framing are confirmed all five routes and remain unfixed. There are9 deliberate negative process faults:5 TOI SIGSEGV and4 GPU-viewer cache SIGILL; the CPU cache input is invalid but survives. None is a clean-quit or feature pass.\n\n388 check evaluations include40 policy failures; original CPU Distance Debug slider-label targeting failure is retained. Only unfinished CPU work ran again. Framing has30 recorded evaluations:5 confirmed failures and5 invalid pan stimulus assumptions (Alt omitted); the original receipts are unchanged. All initial/F/menu capsules remain fully outside independently of the rejected pan assumption. No product change was made.\n\nCurrent totals:89/342 widgets reviewed,253left;113/165 sample dispositions,52left (19actualfixtures+94shared reuse);10/23 custom input groups,13left;16open issues,2earlierlocalfixes. Shared85/85reviewed includes defects/exclusions. PR10 unchecked.\n`);
// Consume only generated streams after selected actual values and original receipts are retained.
const removed:any[]=[];for(const {folder,r} of all)for(const name of ['api.jsonl','observer.jsonl','stderr.txt']){
  const path=join(D,folder,name);if(!existsSync(path))continue;
  const bytes=statSync(path).size,sha256=hash(path);let summary:any;
  if(name==='stderr.txt'){const text=readFileSync(path,'utf8');summary={lines:text.split('\n').length-1,PERF05_lines:(text.match(/PERF05/g)||[]).length,other_lines:text.split('\n').filter(s=>s&&!s.includes('PERF05')).slice(0,8).map(s=>s.slice(0,300))};}
  removed.push({path:relative(D,path),bytes,sha256,summary,reason:'Selected actual operation values/screenshots and original receipt retained in acceptance; consumed full stream'});unlinkSync(path);
}
for(const v of read('manifold-control-build.json').viewers){assert(v.binary.startsWith(join(D,'raw/manifold-control-build/')));if(existsSync(v.binary)){removed.push({path:relative(D,v.binary),bytes:statSync(v.binary).size,sha256:hash(v.binary),reason:'Superseded by qualified five distance-control viewers; manifests/link inputs preserved'});unlinkSync(v.binary)}}
function bytes(p:string):number{if(!existsSync(p))return 0;const st=lstatSync(p);if(st.isSymbolicLink())return 0;return st.isDirectory()?readdirSync(p).reduce((n,f)=>n+bytes(join(p,f)),0):st.size;}
const fs=statfsSync(R),deleted=removed.reduce((n,x)=>n+x.bytes,0);
write('distance-controls-cleanup.json',{status:'CLOSED',removed,deleted_bytes:deleted,owned_pids_absent:pids,owned_core_files:cores,shared_cache_growth_bytes:all.reduce((n,{r})=>n+(r.shared_cache_growth_bytes||0),0),raw_retained_bytes:bytes(join(D,'raw')),free_bytes:Number(fs.bavail)*Number(fs.bsize),retained_consumer:'Five current distance-control viewers for253remainingwidgetentries/13custominputgroups; selectedscreens/receipts/minimalactualvaluesforUI15–UI18repair; existing link objects and CPUReplayfixture retained. Shared caches preserved.'});
console.log({status:'CLOSED_REVIEW_WITH_FINDINGS',checks,findings,crashes,widgets:89,left:253,samples:113,samples_left:52,issues_open:16,verified:2,deleted_bytes:deleted,raw_bytes:bytes(join(D,'raw'))});
