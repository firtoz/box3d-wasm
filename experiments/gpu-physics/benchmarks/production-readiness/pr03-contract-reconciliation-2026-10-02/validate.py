#!/usr/bin/env python3
"""Verify the frozen contract and its original known-failure baseline offline."""
from pathlib import Path
import hashlib,json,re,tarfile,collections
B=Path(__file__).resolve().parent;Q=B.parent
def sha(f):
 h=hashlib.sha256()
 with Path(f).open('rb') as s:
  while data:=s.read(1024*1024):h.update(data)
 return h.hexdigest()
def read(n):return json.loads((B/n).read_text())
d=read('qualification-contract.json')
for n,v in read('raw-index.json').items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
for n,v in d['references'].items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
# Verify actual upstream raw bytes, including failed logs/captures. Earlier
# source-linked offline evaluators remain authoritative for their named scopes.
seen=set();rawfiles=0
for n in d['references']:
 if n.endswith('/raw-index.json'):
  parent=(B/n).parent;idx=json.loads((B/n).read_text())
  for name,v in idx.items():
   f=parent/name
   if str(f.resolve()) in seen:continue
   seen.add(str(f.resolve()))
   if isinstance(v,str):assert sha(f)==v,f
   else:assert sha(f)==v['sha256'] and f.stat().st_size==v['bytes'],f
   rawfiles+=1
assert all(v==0 for v in d['budget'].values())
assert len(d['compiled_inputs'])==len(d['current_inputs'])==107
assert [n for n,h in d['current_inputs'].items() if d['compiled_inputs'][n]!=h]==['src/gpu_invariants.rs']==d['only_uncompiled_fixture_difference']
with tarfile.open(Q/'pr03-rust-baseline-2026-10-02/raw/compiled-source-inputs.tar.gz') as t:
 for n,h in d['compiled_inputs'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
 lib=t.extractfile('src/lib.rs').read().decode();assert '#[cfg(all(test, not(target_arch = "wasm32")))]\nmod gpu_invariants;' in lib
 world=t.extractfile('src/api/world.rs').read().decode();registry=t.extractfile('src/api/world/world_lifetime.rs').read().decode();host=t.extractfile('src/api/world/host_state_trace.rs').read().decode();core=t.extractfile('src/api/world/state_trace.rs').read().decode()
 assert 'struct WorldRegistry' in registry and 'child_ceiling' in registry and 'CURRENT_WORLDS' in registry and 'CURRENT_CHILDREN' in registry
 assert '"schema":"gpu-core-state-v24"' in core and 'WorldRegistry' not in core and 'CURRENT_WORLDS' not in core
 assert 'WorldRegistry' not in host and 'CURRENT_WORLDS' not in host
 assert 'with_joint_metadata(id,|_,_|WorldId{index1:id.world0,generation:1})' in core
 sim=t.extractfile('src/sim.rs').read().decode();assert 'self.params.order_enabled!=0' in re.sub(r'\s+','',sim)
corrected=Q/'pr03-sleeper-fixture-2026-10-02/raw/fixture-builds/candidate-gpu_invariants.rs'
assert sha(corrected)==d['current_inputs']['src/gpu_invariants.rs']
assert 'fn gpu_schedule_uses_body_slot_32768()' in corrected.read_text()
configs={(c['backend'],c['ordering']) for c in d['required_configurations']};assert configs=={(b,o) for b in ['ordinary','native-cached'] for o in [0,1]}
assert all(c['physical_repeat_processes_minimum']==5 and c['full_physical_repeat_status']=='OPEN' for c in d['required_configurations'])
counts=collections.Counter(c['family'] for c in d['baseline_cases']);assert counts=={'API-C':32,'API-Rust':19,'API-host-remaining':1,'Rust':33,'distance':2,'ragdoll-mesh':24,'Rain-initial':2,'drag-order-nativeRain':7,'sleeper':4,'Rain-partial':3},counts
for c in d['baseline_cases']:
 assert c['command'] and re.fullmatch('[0-9a-f]{64}',c['binary_sha256']),c['name']
 assert c['original_result'][c['exit_field']]==c['exit'] and (B/c['protocol_reference']).exists()
 assert 'environment' not in c['source_case']
 for key in ['stdout_sha256','stderr_sha256']:assert re.fullmatch('[0-9a-f]{64}',c['original_result'][key])
p=json.loads((Q/'pr03-rust-baseline-2026-10-02/raw/protocol.json').read_text());r=json.loads((Q/'pr03-rust-baseline-2026-10-02/raw/receipt.json').read_text())
assert r['selected_checks']=={'pass':195,'fail':4,'total':199} or r['selected_checks']==199
assert r['status']=='completed-baseline' and r['all_assertions_pass'] is False
rust=[c for c in d['baseline_cases'] if c['family']=='Rust']
for c,pc,row in zip(rust,p['cases'],r['results'],strict=True):
 assert c['original_result']==row and c['command']==pc['command'] and c['selectors']==pc['selectors']
 assert c['binary_sha256']==p['cells'][pc['cell']]['test_executable']['sha256']
expected={s for c in p['cases'] for s in c['selectors']}|{'gpu_invariants::gpu_schedule_uses_body_slot_32768'}
assert expected==set(d['required_Rust_selectors']) and len(expected)==101
assert len(d['trace_recipes'])==13 and len(d['C_scene_recipes'])==14
assert sorted(c['steps'] for c in d['trace_recipes'])==sorted([600,240,240,120,18,73,73,12,3,16,8,5,48])
assert [(c['kind'],c['poses'],c['argv']) for c in d['C_scene_recipes'] if 'kind' in c]==[('grid',45,[]),('torus',12,[])]
for c in d['C_scene_recipes'][:6]:assert c['argv']==[c['argv'][0],str(c['steps'])]
app=d['selector_applicability'];assert app['api::world::world_counter_tests::contact_order_survives_body_capacity_growth_and_invalidates_proxy_changes']['required_ordering']==[1]
assert app['gpu_invariants::gpu_schedule_uses_body_slot_32768']['steps']==8
assert app['api::world::state_trace::tests::trace_covers_native_full_replay_and_reentry']['required_backend']==['native-cached']
failures=[c for c in d['baseline_cases'] if c['family']=='distance'];assert all(c['exit']==-6 and c['original_result']['first_discrepancy']==['distance-joint','0','1','-1.00004232','-1.00028491'] for c in failures)
rain=[c for c in d['baseline_cases'] if 'rain' in c['name'].lower() and c['family'] in ['Rain-initial','drag-order-nativeRain'] and c['backend']!='cpu'];assert len(rain)==2 and all(c['exit']=='timeout' for c in rain)
sleep=[c for c in d['baseline_cases'] if c['family']=='sleeper'];assert {(c['backend'],c['ordering']) for c in sleep}=={(b,str(o)) for b in ['ordinary','native'] for o in [0,1]} and all(c['exit']==0 for c in sleep)
assert len(d['known_failures'])>=9 and len(d['capture_obligations']['known_omissions_or_unproved'])==5
print(f'Verified127original baseline observations,101required Rust selectors,13trace/14C recipes,four final configurations and{rawfiles}upstream raw files. Failed/incomplete baseline and release/capture gaps remain explicit; zero new engine/build/timing runs.')
