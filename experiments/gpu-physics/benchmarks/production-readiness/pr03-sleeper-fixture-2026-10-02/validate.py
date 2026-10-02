#!/usr/bin/env python3
"""Offline source/build/applicability and four original-limit fixture checks."""
from pathlib import Path
import json,hashlib,tarfile,re
B=Path(__file__).resolve().parent;raw=B/'raw';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();read=lambda n:json.loads((raw/n).read_text());idx=json.loads((B/'raw-index.json').read_text());assert set(idx)=={str(f.relative_to(B)) for f in raw.rglob('*') if f.is_file()}
for n,v in idx.items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
p=read('fixture-builds/protocol.json');b=read('fixture-builds/receipt.json');q=read('permission-repair/protocol.json');r=read('permission-repair/receipt.json');assert b['protocol_sha256']==sha(raw/'fixture-builds/protocol.json') and b['driver_sha256']==sha(raw/'fixture-builds/run.py');assert r['protocol_sha256']==sha(raw/'permission-repair/protocol.json') and r['driver_sha256']==sha(raw/'permission-repair/run.py');assert q['prior_protocol_sha256']==sha(raw/'fixture-builds/protocol.json') and q['prior_receipt_sha256']==sha(raw/'fixture-builds/receipt.json')
prior=json.loads((B.parent/'pr03-drag-order-native-timeout-2026-10-02/raw/receipt.json').read_text());assert prior['status']=='stopped' and not prior.get('running');assert b['prior_campaign_receipt_sha256']==sha(B.parent/'pr03-drag-order-native-timeout-2026-10-02/raw/receipt.json')
assert b['status']=='stopped' and len(b['builds'])==2 and not b['tests'] and 'PermissionError(13' in b['error'];assert p['budget']['test_builds']==2 and p['budget']['test_processes']==4;assert q['budget']['test_builds']==0 and q['budget']['test_processes']==4 and q['cases']==p['cases'];assert all(p['budget'][k]==q['budget'][k]==0 for k in ['production_library_builds','solver_candidates','retries','headline_timing']);assert len(p['candidate_inputs'])==107 and q['candidate_inputs']==p['candidate_inputs'] and b['inputs_before']==p['original_inputs'] and b['inputs_after_edit']==p['candidate_inputs'];assert [n for n,h in p['candidate_inputs'].items() if p['original_inputs'].get(n)!=h]==['src/gpu_invariants.rs']
assert sha(raw/'fixture-builds/original-gpu_invariants.rs')==p['original_inputs']['src/gpu_invariants.rs'];assert sha(raw/'fixture-builds/candidate-gpu_invariants.rs')==p['candidate_inputs']['src/gpu_invariants.rs']
with tarfile.open(raw/'fixture-builds/candidate-source-inputs.tar.gz') as t:
 for n,h in p['candidate_inputs'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
 lib=t.extractfile('src/lib.rs').read().decode();assert p['test_source_cfg_gate']['text'] in lib
baseline=B.parent/'pr03-rust-baseline-2026-10-02/raw';producer=json.loads((baseline/'producers/engine-build.json').read_text());assert producer['inputs_after']==p['original_inputs'] and sha(baseline/'producers/engine-build.json')==p['engine_producer_sha256'];assert p['test_source_cfg_gate']['src/lib.rs_sha256']==p['candidate_inputs']['src/lib.rs']
for build in b['builds']:
 d=raw/'fixture-builds'/(build['backend']+'-build');assert build['exit']==0
 for channel in ['stdout','stderr']:assert sha(d/(channel+'.log'))==build[channel+'_sha256']
 messages=[]
 for line in (d/'stdout.log').read_text().splitlines():
  try:messages.append(json.loads(line))
  except ValueError:pass
 assert any(v==build['compiler_artifact'] for v in messages);assert build['compiler_artifact']['profile']['test'] and build['compiler_artifact']['target']['name']=='gpu_physics';assert build['command']==next(command for name,command in p['recipes'] if name==build['backend']);assert q['binaries'][build['backend']]['sha256']==build['executable_sha256']
assert r['status']=='completed-fixture-checks' and r['all_assertions_pass'] and len(r['permissions'])==2 and len(r['tests'])==4
for v in r['permissions']:
 d=q['binaries'][v['backend']];assert v['mode_before']==d['mode_before'] and not v['mode_before']&0o111;assert v['mode_after']==d['mode_after']==d['producer_mode'] and v['mode_after']&0o111;assert v['binary_sha256_unchanged']==d['sha256']
for c,row in zip(q['cases'],r['tests']):
 assert row['name']==c['name'] and row['exit']==0 and row['pass'] and row['harness_result']==['ok','1','0','0'];assert c['settle_steps']==400 and c['wake_steps']==1 and c['substeps']==4 and c['dt']==1/60;assert c['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']==c['name'][-1];assert row['command']==[q['binaries'][c['backend']]['path'],c['selector'],'--exact','--nocapture','--test-threads=1'];out=raw/'permission-repair'/c['name']
 for channel in ['stdout','stderr']:assert sha(out/(channel+'.log'))==row[channel+'_sha256']
 stderr=(out/'stderr.log').read_text();stdout=(out/'stdout.log').read_text();assert row['actual_NVIDIA_Vulkan'] and 'NVIDIA GeForce RTX 4070 SUPER' in stderr and 'Vulkan' in stderr and 'test result: ok. 1 passed; 0 failed; 0 ignored;' in stdout
 expected='sleeper fixture: stale=BodyId { index1: 7, world0: 1, generation: 1 } live=BodyId { index1: 7, world0: 1, generation: 2 } valid=true asleep=true';assert row['fixture_lines']==[expected] and expected in stderr
old=(raw/'fixture-builds/original-gpu_invariants.rs').read_text();new=(raw/'fixture-builds/candidate-gpu_invariants.rs').read_text();start='fn high_resistance_sleeper_wakes_on_velocity()';end='\nfn overlapping_compound_world';oldprefix,oldrest=old.split(start,1);newprefix,newrest=new.split(start,1);oldbody,oldsuffix=oldrest.split(end,1);newbody,newsuffix=newrest.split(end,1);assert oldprefix==newprefix and oldsuffix==newsuffix
for text in ['for _ in 0..400 {','b3_world_step_gpu(world, 1.0 / 60.0, 4);','assert!(!b3_body_is_awake(sleeper), "fixture must be asleep before testing wakeup");','assert!(b3_body_is_awake(sleeper), "impulse must wake a sleeping capsule");']:assert text in oldbody and text in newbody
aux=read('build-auxiliary.json')
for n,v in aux.items():assert sha(raw/'build-auxiliary'/n)==v['sha256']
assert len({(c['backend'],c['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']) for c in q['cases']})==4
print(f'Validated{len(idx)}rawfiles: two successful exacttestbuilds; original permissionfailure/zero launches retained; four backend×order checks pass original400+1 limits and stale/live identity assertions. No production/default/scene change or final repeat claim.')
