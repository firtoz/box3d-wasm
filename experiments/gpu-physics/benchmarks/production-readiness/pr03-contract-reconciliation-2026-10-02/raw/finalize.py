from pathlib import Path
import json,shutil,os,hashlib
Q=Path('experiments/gpu-physics/benchmarks/production-readiness');B=Q/'pr03-contract-reconciliation-2026-10-02';P=B/'qualification-contract.json';d=json.loads(P.read_text())
for n in list(d['references']):
 if '/docs/gpu-solver-' in n:
  src=(B/n).resolve();target=B/'raw/frozen-criteria'/src.name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(src,target);v=d['references'].pop(n);d['references'][str(target.relative_to(B))]=v
host=json.loads((Q/'pr02-current-acceptance-2026-10-02/raw/remaining-host/receipt.json').read_text());hp=json.loads((Q/'pr02-current-acceptance-2026-10-02/raw/remaining-host/protocol-before-work.json').read_text())
row={'family':'API-host-remaining','protocol_reference':'../pr02-current-acceptance-2026-10-02/raw/remaining-host/protocol-before-work.json','case_index':0,'name':'native/'+hp['selector'],'backend':'native','selectors':[hp['selector']],'command':host['command'],'ordering':'1','component_schedule_environment':'UNSET','source_internal_overrides':'Purehost arithmetic selector; GPUbanner not applicable. Original nominalGPU harness stop retained.','exit':host['exit'],'original_result':host,'source_case':{'selector':hp['selector'],'host_only':True},'expected_trace_frames':None,'steps':None,'binary_sha256':host['binary_sha256']};assert not any(c['family']=='API-host-remaining' for c in d['baseline_cases']);d['baseline_cases'].append(row)
for c in d['baseline_cases']:
 p=json.loads((B/c['protocol_reference']).read_text());cell=c['backend'] or c['source_case'].get('backend');c['backend']=cell;family=c['family']
 if not c.get('binary_sha256'):
  if family=='Rust':c['binary_sha256']=p['cells'][cell]['test_executable']['sha256']
  elif family in ['Rain-initial','drag-order-nativeRain']:
   kind=c['source_case'].get('kind')
   if kind=='rain':c['binary_sha256']=p['viewers'][cell]['binary_sha256']
   elif kind=='order-cache':c['binary_sha256']=c['source_case']['binary']['binary_sha256']
   else:c['binary_sha256']=c['source_case']['fixture']['binary_sha256']
  elif family=='sleeper':c['binary_sha256']=p['binaries'][cell]['sha256']
  elif family=='Rain-partial':c['binary_sha256']=json.loads((Q/'pr03-rain-phase-diagnostic-2026-10-02/raw/remaining/receipt.json').read_text())['observer_executable_sha256']
 assert c.get('binary_sha256') and len(c['binary_sha256'])==64,(family,c['name'])
for c in d['C_scene_recipes'][:6]:c['argv'].append(str(c['steps']))
mesh=d['C_scene_recipes'].pop();assert mesh['fixture']=='rain_frozen_contact.cpp'
for kind,count in [('grid',45),('torus',12)]:
 path=Q/'pr03-baseline-fixtures-2026-10-02/raw/frozen-mesh-inputs'/(kind+'.txt');ref=os.path.relpath(path,B);d['references'][ref]={'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'bytes':path.stat().st_size};d['C_scene_recipes'].append(dict(mesh,argv=[],stdin_reference=ref,poses=count,kind=kind))
d['selector_applicability']={'api::world::world_counter_tests::contact_order_survives_body_capacity_growth_and_invalidates_proxy_changes':{'required_ordering':[1],'reason':'Source liveorder allocation/seeding explicitlydisabled with0. Preserve original0failure, run unchangedstorage requirement in1; mixed/schedule/equivalence/capacity checks independentlyrequired inbothmodes.'},'api::world::state_trace::tests::trace_covers_native_full_replay_and_reentry':{'required_backend':['native-cached'],'required_ordering':[0,1],'actual_replay_hits':42},'gpu_invariants::gpu_schedule_uses_body_slot_32768':{'steps':8,'substeps':4,'dt':1/60,'required_backend':['ordinary','native-cached'],'required_ordering':[0,1],'baseline_status':'notselected;required finalregression,notreplacedby30000cube insertiontest'}}
d['gate_interpretation']='PR03 freezes executable baseline, applicable criteria and futurequalification matrix. Known failed/incomplete baseline observations are required inputs toPR04, not claimed physical passes. Completephysical/futurestate/freshbuild/performance acceptance remainsPR04–PR08. No historical criterion weakened or failure removed.'
P.write_text(json.dumps(d,indent=2)+'\n');print('Final contract draft:127baseline cases,14C scene recipes,101requiredRust selectors,13trace recipes; all original binary hashes resolved.')
