from pathlib import Path
import hashlib,json,os,subprocess,time
R=Path.cwd();D=Path(__file__).resolve().parent;start=time.monotonic();labels=['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'];checks=[];removed={};examples=[];cache=[]
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
for label in labels:
 before=json.loads((D/'raw/ui01-before-runs'/label/'receipt.json').read_text());tag='ordinary-both-stat-safe'if label=='ordinary-both'else label;C=D/'raw/ui01-after-runs'/tag;p=C/'receipt.json';after=json.loads(p.read_text());assert len(before['checks'])==len(after['checks'])==20 and all(after['checks'].values())and before['checks']==after['checks'];assert after['app_terminal_exit']==0 and not Path('/proc/'+str(after['app_pid'])).exists();assert all(not x['clipped_rows']for x in after['UI01_clip_observations'])
 preserved=['world','body_count','shape_count','hertz','substeps','context_flags','world_flags','cpu_world','cpu_flags']
 for name,v in after['screenshots'].items():
  assert sha(v['path'])==v['sha256'];old=before['screenshots'][name]
  for field in preserved:assert old['state'].get(field)==v['state'].get(field),(label,name,field)
 if label!='cpu':
  fn=lambda phase,tag:json.loads((D/('raw/ui01-'+phase+'-runs')/tag/'initial1600.ui.json').read_text())
  a=next(x for x in fn('before',label)['items']if x['label']=='CPU draw = import+pose+list; do not add to GPU device');b=next(x for x in fn('after',tag)['items']if x['label']==a['label']);assert a['rect'][2]>a['clip'][2]and b['rect'][2]<=b['clip'][2]+.5
  examples.append({'label':label,'same_text':a['label'],'before_rect':a['rect'],'after_rect':b['rect'],'before_clip':a['clip'],'after_clip':b['clip'],'before_width':a['rect'][2]-a['rect'][0],'after_width':b['rect'][2]-b['rect'][0]})
 after['visual_review']={'status':'PASS_NINE_AFTER_STAGES','inspection':'CPUfirstInfo andgeometry grids coverallnine afterstages; intendedHull/Capsulechanges, checkbox/restart/counters/layout preserved.9→0 native textcliprows.','limits':'ShortpauseddefaultUI subset only; allothercontrols and fullphysics/repeatability/performance remainunqualified.'};p.write_text(json.dumps(after,indent=2)+'\n');checks.append({'label':label,'before_checks':20,'after_checks':20,'original_images':18,'source_scoped_identity_defaults_preserved':preserved,'after_seconds':after['elapsed_seconds']});cache.append({'label':label,'before':after['shared_cache_before'],'after':after['shared_cache_after']})
build=json.loads((D/'ui01-build-after.json').read_text());assert all(x['only_scoped_wrap_pair_added']and x['current_production_generator_Info_function_exact']for x in build['source_preservation'])
for f,h in build['input_hashes'].items():assert sha(f)==h,f
for x in build['source_preservation']:
 assert sha(D/'raw/ui01-after'/x['mode']/'sample_bench.cpp')==x['candidate_source_sha256'];assert sha(D/'raw/ui01-after'/x['mode']/'sample_bench.cpp.o')==x['candidate_object_sha256']
assert subprocess.check_output(['git','-C','box3d','status','--porcelain'])==b''
assert subprocess.check_output(['git','diff','--name-only','--','experiments/gpu-physics/src','experiments/gpu-physics/shaders','experiments/gpu-physics/c_abi','packages/box3d-wasm','demo/public/wasm'])==b''
# Preserve only event windows needed for the tested transitions, including failures.
stream_summaries=[]
for C in list((D/'raw/ui01-before-runs').iterdir())+list((D/'raw/ui01-after-runs').iterdir()):
 p=C/'observer.jsonl'
 if not p.exists():continue
 r=json.loads((C/'receipt.json').read_text());states=[json.loads(l)for l in p.read_text().splitlines()];assert states
 assert all(s['context_flags']==s['world_flags'] for s in states),C
 assert all('cpu_flags'not in s or s['cpu_flags']==s['context_flags']for s in states),C
 events=[]
 for a in r['actions']:
  if a['kind']=='key period':
   lo=a['state_before']['frame'];hi=a['state_after']['frame'];w=a['state_after']['world'];rows=[s for s in states if lo<s['frame']<=hi and s['world']==w];moved=[s for s in rows if s['move_count']>0];events.append({'frame_window':[lo,hi],'world':w,'records':len(rows),'moved_records':len(moved),'first_moved':moved[0]if moved else None})
 stream_summaries.append({'run':str(C.relative_to(D)),'status':r['status'],'records':len(states),'stream_sha256':sha(p),'flags_every_record_match':True,'both_CPU_flags_every_record_match':True,'step_event_windows':events});removed[str(p.relative_to(D))]=p.stat().st_size;p.unlink()
(D/'ui01-stream-summaries.json').write_text(json.dumps(stream_summaries,indent=2)+'\n')
for label in labels:
 f=D/'raw/ui01-before'/label;removed[str(f.relative_to(D))]=f.stat().st_size;f.unlink()
result={'status':'PASS_UI01_VERIFIED_LOCAL','fixed_issue':'UI01','full_PR10_complete':False,'native_inventory_mapped':165,'full_interaction_reviews_done':0,'full_interaction_reviews_left':165,'checks_before':100,'checks_after':100,'original_before_after_images':90,'after_text_clip_rows_all_stages':0,'same_input_layout_examples':examples,'routes':checks,'source_preservation':build['source_preservation'],'shared_cache_observations':cache,'shared_cache_causality':'Measuredsharedroots only; concurrentotherapp activitycannotbe attributed tothisbatch. Preservedallsharedcache.','consumed_temporary_bytes_deleted':sum(removed.values()),'deleted_files':removed,'current_after_diagnostic_viewers_retained':'5currentlyqualifiedviewers/objects forremainingPR10includingUI02 repro; removewhenreplaced/goalends','free_bytes_after':os.statvfs(R).f_bavail*os.statvfs(R).f_frsize,'seconds':time.monotonic()-start};(D/'ui01-acceptance.json').write_text(json.dumps(result,indent=2)+'\n')
issues=json.loads((D/'issues.json').read_text());issues[0].update(status='FIXED_VERIFIED_LOCAL',repair='ScopedPushTextWrapPos/PopTextWrapPos in generatedInfoPanel; samephysicalsource outsidewindow/currentgeneratorfunctionexact. 100before+100after checks, all5routes/90originalstages, clip9→0. Notcommitted; fullaffected-scene precommit requirement and remainingPR10 stayopen.');(D/'issues.json').write_text(json.dumps(issues,indent=2)+'\n')
(D/'UI01.md').write_text('''# UI01: readable Info panel text

[x] Verified locally. CPU first; ordinary/native standalone and combined routes pass. PR10 remains open: full sample interaction reviews **0 done / 165 left**.

The fixed-width Info panel clipped long diagnostic text and the GPU recording-unavailable explanation. Actual ImGui item rectangles exceeded its clip bounds. The generator now scopes text wrapping to this window and restores the wrapping stack before leaving it. The exact text, diagnostics, default expansion and all code outside this window are preserved. Box3D/WASM, Rust/WGSL, geometry, defaults and body/picking code are unchanged.

Before, actual Box3D CPU first:

![Before: GPU text clipped](ui01-before-info-grid.jpg)

After, actual Box3D CPU first:

![After: complete wrapped text](ui01-after-info-grid.jpg)

[Original CPU-first comparison grid](ui01-compare.html), [acceptance and same-input rectangle measurements](ui01-acceptance.json), [compact world/event checks](ui01-stream-summaries.json).

All five routes pass 20 checks before and after: pause/single-step, actual context/world flags, restart lifetime and retained settings, metrics, sample selection, Jenga Hull/Capsule changes, resize and clean quit. All 90 saved stages were visually inspected. Native before captures have 7–9 clipped text rows; after captures have zero at every tested stage. World IDs, populations, hertz/substeps and flags agree before/after for the selected stages. Every sampled combined CPU flag agrees too. These are scoped checks, not whole-physics, full-state, manual-picking or all-widget qualification.

The shared parent Info rendering affects all165 registrations by source. This report executes Single Box/Jenga only; other sample-specific widgets, child windows, layouts and interactions remain unreviewed. Required broader affected-scene snapshots must precede a future commit; no PR10 commit/push is newly authorized. UI02 panel-wheel capture needs reproduction; startup/cache limitations remain explicit PR05/PR08 inputs. Original harness failures and their bounded corrections are retained, not promoted to passes. No new engine build or physics experiment was run.

Consumed observer streams were reduced to required checks/event windows; five before executables were deleted. The five after diagnostic viewers remain needed for remaining PR10 checks. Shared driver/app caches were measured and preserved.
''')
p=D/'log.md';s=p.read_text();i=s.index('\n\n')+2;p.write_text(s[:i]+f'2026-10-15 UI01[x] VERIFIED LOCAL:100before+100afterchecks/all5routes/90originalstages inspected, nativeInfo7–9clippedrows→0allafter. Sameconstanttext actualImGui boundingcalculation traced; scopedwrap only in Info, outsidecode preserved/productiongeneratedfunctionexact3variants. WorldIDs/counts/default hertz60/substeps4/flags preserved; everycombinedCPUflags sampledmatch. NoRust/WGSL/Box3D/WASM orpickingcode changes. {sum(removed.values())}consumedstream/beforebinarybytesdeleted; fiveaftertraceviewers retained forremainingPR10/UI02. Sharedcachemeasured/preserved. Session86967terminal0; allappsreaped/budgetsclosed. UI01report UI01.md/acceptance/CPUfirstbeforeaftergrid; notcommitted, broadallaffectedscenesprecommitstillrequired. Fullinteractionreviews0done/165left; inventory165mapped/0left; onefixUI01, UI02input-boundaryrepropending, startupPR05/08limitation. NEXT readlogfirst, traceactualrapidUIwheel event/capture/camerainputs withrealCPUfirst underNEWfinitebudget; reusecurrentqualifiedafterviewers/source, nooldmanualreceiver/cold/solvercampaign.\n\n'+s[i:])
p=R/'docs/goals/gpu-production-readiness.md';s=p.read_text();a=s.index('Current PR10 interaction checks:');b=s.index('\n',a);s=s[:a]+'Current PR10 interaction reviews: 0 complete / 165 remaining. The source inventory maps all165 samples (342 sample widget call sites,84shared); the first shared subset and Jenga widgets pass on all5 routes. [UI01 Info text wrapping](../../experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15/UI01.md) is verified locally with CPU-first before/after evidence,100before+100afterchecks and unchanged engine sources. UI02 panel-wheel/camera coupling needs input-boundary reproduction; startup latency remains PR05/PR08. PR10 stays unchecked; no new commit/push authorization. Read the PR10 log for exact retained after-viewer paths, finite budgets and cleanup. All UI01 jobs are terminal; do not restart closed/manual/cold/solver experiments.'+s[b:];p.write_text(s)
print(json.dumps({k:v for k,v in result.items()if k in ['status','checks_before','checks_after','original_before_after_images','after_text_clip_rows_all_stages','consumed_temporary_bytes_deleted','free_bytes_after','seconds']}))
