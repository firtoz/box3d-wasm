from pathlib import Path
import json,hashlib,subprocess,re,os,gzip
R=Path.cwd();E=R/'experiments/gpu-physics';Q=E/'benchmarks/production-readiness';A=E/'artifacts/production-readiness/pr03-contract-reconciliation';B=Q/'pr03-contract-reconciliation-2026-10-02';assert not B.exists();B.mkdir()
def sha(f):
 h=hashlib.sha256()
 with Path(f).open('rb') as s:
  while data:=s.read(1024*1024):h.update(data)
 return h.hexdigest()
refs={}
def ref(f):
 f=Path(f);n=os.path.relpath(f,B);refs[n]={'sha256':sha(f),'bytes':f.stat().st_size};return n
def doc(report,n):
 path=Q/report/n;ref(path);return json.loads(path.read_text())
# Keep complete original datasets; this is a linked reconciliation, not another run.
reports=['pr01-current-acceptance-2026-10-02','pr02-current-acceptance-2026-10-02','pr03-baseline-fixtures-2026-10-02','pr03-rust-baseline-2026-10-02','pr03-rain-launch-timeout-2026-10-02','pr03-drag-order-native-timeout-2026-10-02','pr03-sleeper-fixture-2026-10-02','pr03-rain-phase-diagnostic-2026-10-02']
for report in reports:
 for n in ['README.md','raw-index.json','validate.py','acceptance.json']:
  path=Q/report/n
  if path.is_file():ref(path)
 index=json.loads((Q/report/'raw-index.json').read_text())
 for n in index:
  path=Q/report/n
  if path.suffix in ['.json','.py','.rs','.cpp','.h'] and not any(x in n for x in ['applicable-inventory','build-auxiliary']):ref(path)
ref(Q/'pr03-rust-baseline-2026-10-02/release-contract.md')
for n in ['raw/receipt.json','raw/compiled-source-inputs.tar.gz','raw/ordinary-gpu/receipt.json','raw/native-gpu/receipt.json','raw/ordinary-both/receipt.json','raw/native-both/receipt.json','raw/ordinary-gpu/generated-inputs.tar.gz','raw/native-gpu/generated-inputs.tar.gz']:
 ref(Q/'pr02-world-lifetime-viewer-builds-2026-10-02'/n)
for n in ['docs/gpu-solver-qualification.md','docs/gpu-solver-goal.md']:ref(R/n)
ref(Q/'pr03-rust-baseline-2026-10-02/raw/compiled-source-inputs.tar.gz')
source=doc('pr03-rust-baseline-2026-10-02','raw/producers/engine-build.json')['inputs_after'];current={n:sha(E/n) for n in source};assert [n for n in source if source[n]!=current[n]]==['src/gpu_invariants.rs']
corrected=Q/'pr03-sleeper-fixture-2026-10-02/raw/fixture-builds/candidate-gpu_invariants.rs';assert current['src/gpu_invariants.rs']==sha(corrected)
records=[]
def add(family,protocol_path,cases,results,command_key='command'):
 by_name={v.get('name'):v for v in results if v.get('name')}
 for i,c in enumerate(cases):
  row=by_name.get(c.get('name'),results[i]);env=c.get('environment',row.get('environment',{}));command=c.get(command_key,row.get('command'))
  if command is None and 'fixture' in c and isinstance(c['fixture'],dict):command=[c['fixture']['binary'],*c.get('args',[])];command.extend([c['scene']] if family=='ragdoll-mesh' else [])
  binary_hash=row.get('binary_sha256') or c.get('fixture',{}).get('binary_sha256') if isinstance(c.get('fixture',{}),dict) else row.get('binary_sha256')
  record={'family':family,'protocol_reference':protocol_path,'case_index':i,'name':c.get('name',row.get('fixture',row.get('selector',str(i)))),'backend':c.get('cell',row.get('cell',row.get('backend'))),'selectors':c.get('selectors',[row['selector']] if row.get('selector') else []),'command':command,'ordering':env.get('GPU_PHYSICS_LIVE_CONTACT_ORDER','source/default'),'component_schedule_environment':env.get('GPU_PHYSICS_COMPONENT_TGS','UNSET'),'source_internal_overrides':'Source assertions/fixtures may intentionally choose scheduling/CCD/residency; command environment alone does not override these choices.','exit':row.get('exit'),'original_result':row,'source_case':{k:v for k,v in c.items() if k!='environment'}}
  record['expected_trace_frames']=c.get('expected_trace_frames');record['steps']=c.get('steps');record['binary_sha256']=binary_hash;records.append(record)
# Current API baseline is CPU-compatible1; host-only selectors classified separately.
pr2='pr02-current-acceptance-2026-10-02';p2=doc(pr2,'raw/protocol-before-work.json');r2=doc(pr2,'raw/receipt.json')
for row in r2['C_runs']:
 c=dict(row);c['name']=row['cell']+'/'+row['fixture'];add('API-C',ref(Q/pr2/'raw/protocol-before-work.json'),[c],[row])
for row in r2['Rust_runs']:
 c=dict(row);c['name']=row['backend']+'/'+row['selector'];add('API-Rust',ref(Q/pr2/'raw/protocol-before-work.json'),[c],[row])
host=doc(pr2,'raw/remaining-host/receipt.json')
for row in host.get('results',host.get('Rust_runs',[])):
 c=dict(row);c['name']=row.get('backend','native')+'/'+row.get('selector','remaining-host');add('API-host-remaining',ref(Q/pr2/'raw/remaining-host/protocol-before-work.json'),[c],[row])
for family,report,pn,rn in [
 ('Rust','pr03-rust-baseline-2026-10-02','raw/protocol.json','raw/receipt.json'),
 ('distance','pr03-baseline-fixtures-2026-10-02','raw/distance/protocol.json','raw/distance/receipt.json'),
 ('ragdoll-mesh','pr03-baseline-fixtures-2026-10-02','raw/ragdoll-mesh/protocol.json','raw/ragdoll-mesh/receipt.json'),
 ('Rain-initial','pr03-rain-launch-timeout-2026-10-02','raw/display-repair/protocol.json','raw/display-repair/receipt.json'),
 ('drag-order-nativeRain','pr03-drag-order-native-timeout-2026-10-02','raw/protocol.json','raw/receipt.json'),
 ('sleeper','pr03-sleeper-fixture-2026-10-02','raw/permission-repair/protocol.json','raw/permission-repair/receipt.json')]:
 p=doc(report,pn);r=doc(report,rn);results=r.get('results',r.get('tests'));cases=p['cases'][:len(results)];add(family,ref(Q/report/pn),cases,results)
phase='pr03-rain-phase-diagnostic-2026-10-02';p=doc(phase,'raw/original/protocol.json');r=doc(phase,'raw/remaining/receipt.json');add('Rain-partial',ref(Q/phase/'raw/original/protocol.json'),p['cases'],[r['retained_control'],*r['results']])
# Original full test selection is retained verbatim; add the existing omitted slot
# regression to the final required selection instead of silently substituting30k.
rust=doc('pr03-rust-baseline-2026-10-02','raw/protocol.json');names=sorted({s for c in rust['cases'] for s in c['selectors']});slot='gpu_invariants::gpu_schedule_uses_body_slot_32768';assert slot not in names;assert 'fn gpu_schedule_uses_body_slot_32768()' in (E/'src/gpu_invariants.rs').read_text()
configs=[{'backend':backend,'ordering':order,'device':'NVIDIA Vulkan / RTX4070SUPER /610.57.04','full_physical_repeat_status':'OPEN','physical_repeat_processes_minimum':5,'final_binary':'PR06 exact compiled-source and executable receipt required; original baseline binaries are not final builds'} for backend in ['ordinary','native-cached'] for order in [0,1]]
trace_recipe=[{'selector':c['selectors'][0],'steps':c['expected_trace_frames'],'command':['${PR06_TEST_BINARY}',c['selectors'][0],'--exact','--nocapture','--test-threads=1'],'capture':'GPU_PHYSICS_TEST_TRACE=unique-file; audited completed-step full semantic capture required','scope':'bothbackends/order0&1; native fullreplay only nativebackend'} for c in rust['cases'] if c['cell']=='ordinary' and c.get('expected_trace_frames')]
replay=next(c for c in rust['cases'] if c['name'].endswith('trace-trace_covers_native_full_replay_and_reentry'));trace_recipe.append({'selector':replay['selectors'][0],'steps':48,'command':['${PR06_TEST_BINARY}',replay['selectors'][0],'--exact','--nocapture','--test-threads=1'],'capture':'unique audited complete-state file','scope':'native-cached/order0&1;42actual replayhits required'})
physical_c=[{'fixture':'ragdoll_reference.cpp','argv':[scene], 'steps':600 if scene=='ragdolls' else 60 if scene=='ragdolls-no-contacts' else 120,'configurations':'independent CPU + ordinary/native ×order0/1','limits':'Original source and release-contract table, no relaxed trajectory/physical/isolated limits'} for scene in ['ragdolls','ragdolls-no-contacts','twist','twist-negative','tilted','tilted-negative']]
physical_c += [
 {'fixture':'Benchmark/Rain','argv':['--sample-name','Benchmark/Rain','--unpaced','--health-scan','--hide-ui','--sleep','--workers','8','--warmup','0','--timed','600','--bench-json','${UNIQUE_CAPTURE}'],'steps':600,'configurations':'independent CPU + ordinary/native ×order0/1','limits':'Original capture/creation/recycling/typed parameter/+.05m,+.05rad residual contract; fullstate also required'},
 {'fixture':'both_drag_test.cpp','argv':[], 'steps':1320,'configurations':'combined independent CPU + ordinary/native ×order0/1','limits':'Original isolated/endpoint checks'},
 {'fixture':'both_drag_test.cpp','argv':['--ground'],'steps':3060,'configurations':'combined independent CPU + ordinary/native ×order0/1','limits':'Original loaded/perframe/endpoint checks; no resets'},
 {'fixture':'distance-original.cpp','argv':[], 'steps':240,'configurations':'combined independent CPU + ordinary/native ×order0/1','limits':'Four original sphere-ground/distance ×1/4substep cases, all13lanes absolute1e-5;20/60/100/160 toggles'},
 {'fixture':'both_joint_reaction_test.cpp','argv':[], 'steps':8,'configurations':'combined independent CPU + ordinary/native ×order0/1','limits':'Original ninekind×fourconfiguration relative1e-5 comparisons,1/60→1/120,3/4substeps and original transitions'},
 {'fixture':'both_joint_separation_test.cpp','argv':[], 'steps':'Original frozenpose queries/mutations','configurations':'combined independent CPU + ordinary/native ×order0/1','limits':'Original ninekind relative1e-5 comparisons; upstream wheel angular unavailable/zero semantics remain disclosed'},
 {'fixture':'rain_frozen_contact.cpp','argv':['${GRID_OR_TORUS_INPUT}'],'steps':'45grid/12torus original poses','configurations':'independent CPU + ordinary/native ×order0/1','limits':'Original1e-5contact/separation/normal/count assertions; preserve exact input hashes'}]
source_audit={'present_capture':'schema24 core/contact/allocation/hostgeometry/policy/callback/replay groups as archived sources; public health/pose CSV is only a subset','mandatory_registry_fields':['WorldRegistry.slots live/root identity and meaningful slotorder','WorldRegistry.epochs world/child_ceiling including retired/exhausted slots','CURRENT_WORLDS/CURRENT_CHILDREN relational epochs under consistent registry snapshot','WorldInner.generation/child_generation/body_generations/free slots/creation ordinals'],'mandatory_ABI_fields':['C shape/body/joint/geometry/publicID mappings and generation routing','GpuSlots capacity/high_water/allocated chunks/live slots and holes affecting future allocation/failure','WorldVis callbacks and context semantics plus ShapeVis geometry ownership','sticky errors/invalidstate/lifetime cleanup/combined CPU mappings; external callback code/context identities and actual decisions/order'],'known_omissions_or_unproved':['Current core capture does not serialize WorldRegistry.epochs/root/child epoch arrays','No full C metadata serializer in Rust world capture','Diagnostic spherical/revolute cache setters use generation1 after metadata validation; recreated worlds unqualified','Rust world indices extend to65535 while C metadata accepts1..63; range/failure reconciliation remainsPR05','Compact/lossless fullsemantic coverage and corruption/negative controls stillPR07'],'required_rules':['No meaningful order sorting or float rounding','Bitexact floats and relational identities preserved','Incidental timestamps/padding/opaque handles excluded only by audited source consumers','Five freshfull-duration repeats/configuration; interrupted/pose-only/historical/shortwindows cannot pass','Current cached builds never substitute for PR06 clean consumer builds']}
obj={'schema':'gpu-production-qualification-contract-v1','status':'fixedcontract and knownfailure baseline; NOT engine/release qualification','objective_reference':'../../../../../docs/goals/gpu-production-readiness.md','frozen_at_revision_context_only':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'budget':{'new_engine_processes':0,'new_builds':0,'new_GPU_diagnostics':0,'new_candidates':0,'retries':0,'headline_timing':0},'compiled_production_source_milestone':'abc0a54a3ce3b285c9984f2c7c6b5baefff00d1b','compiled_inputs':source,'current_inputs':current,'only_uncompiled_fixture_difference':['src/gpu_invariants.rs'],'required_configurations':configs,'baseline_cases':records,'required_Rust_selectors':sorted([*names,slot]),'trace_recipes':trace_recipe,'C_scene_recipes':physical_c,'original_limits_and_geometry_source':ref(Q/'pr03-rust-baseline-2026-10-02/release-contract.md'),'original_CPU_oracle':'Full independent Box3D archiveb35f71d07515e333fa19f3e6297bbfc2040984a49f366c94afc5f974acef2c0c;119actual compiled source/object receipt pairs in baselinefixture report','capture_obligations':source_audit,'known_failures':['Both original distance firststep/lane1 absolute1e-5 failure beforewarmstart toggles; later branchnotrun','Both original strict ragdoll first60frame0.006m trajectory screen fails0.063585767m; physical600screen passes separately','Both loadeddrag3060 originalheldposition/totalpose/velocity screens fail,firstheldpositionframe227/body0;releaseendpoints pass separately','Ordinary/native600Rain900second watchdogs/nohealthfile remain incomplete','Current ordinary180Rain residualscreens failfirststep107; no recycling or finalpass','Original order0contactorder-storage assertion fails; sourceapplies toorder1 and unchangedorder1controls pass','Original hardcodedgeneration1sleeper suite failures retained; correctedcfg(test)fixture passesfour backend×order checks','Both exhausted scheduling campaigns remain closed; mixedtarget unmet','Required order1fullscene/fiveprocess/fullstate/cleanbuild/performance/presentation qualification notperformed'], 'future_experiment_rule':'Before every candidate/build/device/diagnostic/timing campaign freeze exactinputs/provenance/configurationorder/settings/watchdog/finitebudget/stopretainrule. Allbadresults retained; no unchanged retries/watchdog extensions/favorable-only expansion. NewPR06inputs refreeze allaffected evidence. Existing exhausted budgets remain closed.','references':refs}
(B/'qualification-contract.json').write_text(json.dumps(obj,indent=2)+'\n');print(json.dumps({'baseline_case_count':len(records),'required_Rust_selectors':len(obj['required_Rust_selectors']),'trace_recipes':len(trace_recipe),'C_scene_recipes':len(physical_c),'references':len(refs)},indent=2))
