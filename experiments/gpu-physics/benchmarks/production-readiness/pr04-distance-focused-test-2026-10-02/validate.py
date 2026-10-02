#!/usr/bin/env python3
"""Verify focused-regression evidence offline; no GPU, binary or host paths needed."""
from pathlib import Path
import hashlib,json,re,tarfile
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def read(n):return json.loads((R/n).read_text())
idx=read('raw-index.json')
for n,v in idx.items():assert sha(R/n)==v['sha256'] and (R/n).stat().st_size==v['bytes'],n
p=read('raw/protocol.json');s=read('raw/receipt.json')
assert p['budget']=={'test_builds':2,'fresh_test_processes':4,'test_selectors_per_process':1,'solver_candidates':0,'library_builds':0,'retries':0,'timing':0}
assert s['status']=='completed-focused-test' and s['all_assertions_pass'] and not s.get('running')
assert s['protocol_sha256']==sha(R/'raw/protocol.json') and s['driver_sha256']==sha(R/'raw/run.py')
assert s['compiled_inputs_before']==s['compiled_inputs_after']==p['compiled_inputs'] and len(p['compiled_inputs'])==107
refs=read('reference-map.json')
for k in ['prior_producer','prior_regression_protocol']:assert refs[p[k]]['sha256']==p[k+'_sha256']==sha(R/refs[p[k]]['portable'])
prior=read('raw/reference/clamp-producer.json')
assert [n for n in p['compiled_inputs'] if p['compiled_inputs'][n]!=prior['compiled_inputs_after'][n]]==p['changes_from_retained_producer']==['src/gpu_invariants.rs']
assert s['native_generated_inputs']==prior['native_generated_inputs'] and len(s['native_generated_inputs'])==455
with tarfile.open(R/'raw/source-inputs.tar.gz') as t:
 for n,h in p['compiled_inputs'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
 source=t.extractfile('src/gpu_invariants.rs').read().decode();lib=t.extractfile('src/lib.rs').read().decode()
 assert '#[cfg(all(test, not(target_arch = "wasm32")))]\nmod gpu_invariants;' in lib
 assert 'fn distance_joint_one_substep_matches_cpu_reference()' in source
 assert 'const CPU_REFERENCE_Y: f32 = -1.00028491;' in source and 'const ABSOLUTE_TOLERANCE: f32 = 1e-5;' in source
 assert 'fn distance_joint_box_stays_bounded()' in source
baseline=read('raw/reference/original-distance-failure.json')
for x in baseline['results']:
 assert not x['completed_fixture'] and x['first_discrepancy']==['distance-joint','0','1','-1.00004232','-1.00028491']
 assert abs(float(x['first_discrepancy'][3])-float(x['first_discrepancy'][4]))>1e-5
assert len(s['builds'])==2 and len(s['results'])==4
for b in s['builds']:
 assert b['exit']==0 and b['stdout_sha256']==sha(R/'raw'/b['name']/'stdout.log') and b['stderr_sha256']==sha(R/'raw'/b['name']/'stderr.log')
for c,x in zip(p['cases'],s['results'],strict=True):
 assert x['command']==c['command'] and x['exit']==0 and x['pass'] and x['harness_result']==['ok','1','0','0'] and x['actual_NVIDIA_Vulkan']
 assert x['binary_sha256']==s['test_binaries'][c['backend']]['sha256'] and x['environment_overrides']==c['environment_overrides']
 so=R/'raw'/c['name']/'stdout.log';se=R/'raw'/c['name']/'stderr.log';assert sha(so)==x['stdout_sha256'] and sha(se)==x['stderr_sha256']
 assert 'NVIDIA GeForce RTX 4070 SUPER' in se.read_text() and 'backend=Vulkan' in se.read_text()
 m=re.search(r'GPU Y=([-0-9.]+), CPU Y=([-0-9.]+), abs_error=([-0-9.]+)',so.read_text());assert m
 gy,cy,err=map(float,m.groups());assert cy==float(baseline['results'][0]['first_discrepancy'][4]) and abs(gy-cy)<=1e-5 and err==0
 assert c['command'][1]=='gpu_invariants::distance_joint_one_substep_matches_cpu_reference'
print(f'PASS:{len(idx)} raw files,107 test inputs,455 unchanged native dependencies,two builds/four one-step NVIDIA Vulkan passes. Original negative control retained;no production/performance/release change.')
