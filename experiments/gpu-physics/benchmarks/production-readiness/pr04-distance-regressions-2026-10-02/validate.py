#!/usr/bin/env python3
"""Keep actual failures distinct from pre-frozen mode applicability."""
from pathlib import Path
import json,hashlib,tarfile,re
B=Path(__file__).resolve().parent
def read(n):return json.loads((B/n).read_text())
def sha(f):return hashlib.sha256(Path(f).read_bytes()).hexdigest()
for n,v in read('raw-index.json').items():assert sha(B/n)==v['sha256'] and (B/n).stat().st_size==v['bytes'],n
p=read('raw/protocol.json');r=read('raw/receipt.json');a=read('acceptance.json');contract=read('raw/pre-candidate-contract.json');prior=read('raw/prior-clamp-receipt.json')
assert p['budget']=={'test_builds':2,'fresh_test_processes':4,'selected_assertion_checks':244,'new_solver_candidates':0,'library_builds':0,'retries':0,'headline_timing':0}
assert r['protocol_sha256']==sha(B/'raw/protocol.json') and r['driver_sha256']==sha(B/'raw/run.py') and p['prior_receipt_sha256']==sha(B/'raw/prior-clamp-receipt.json')
assert prior['all_original_assertions_pass'] and prior['production_shader_restored'] and prior['compiled_inputs_after']==p['candidate_inputs']
assert r['status']=='completed-regressions' and r['production_shader_restored'] and r['all_assertions_pass'] is False and not r.get('running')
assert r['compiled_inputs_before']==r['compiled_inputs_after']==p['candidate_inputs'] and r['original_inputs_after']==p['original_inputs']==contract['current_inputs']
assert [n for n,h in p['original_inputs'].items() if p['candidate_inputs'][n]!=h]==['shaders/physics/solve.wgsl']
assert r['native_generated_inputs']==prior['native_generated_inputs']
with tarfile.open(B/'raw/candidate-source-inputs.tar.gz') as t:
 for n,h in p['candidate_inputs'].items():assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==h,n
assert len(r['builds'])==2 and all(v['exit']==0 for v in r['builds'])
for build,recipe in zip(r['builds'],p['builds'],strict=True):
 assert build['command']==recipe['command'] and build['stdout_sha256']==sha(B/'raw'/build['name']/'stdout.log') and build['stderr_sha256']==sha(B/'raw'/build['name']/'stderr.log')
 v=r['test_binaries'][recipe['backend']];assert v['compiler_artifact']['executable'] and len(v['sha256'])==64
 assert ('native-command-cache' in v['compiler_artifact']['features'])==(recipe['backend']=='native')
name='api::world::world_counter_tests::contact_order_survives_body_capacity_growth_and_invalidates_proxy_changes'
assert contract['selector_applicability'][name]['required_ordering']==[1]
applicable=0;failed=0
for row,c in zip(r['results'],p['cases'],strict=True):
 assert row['command']==c['command'] and row['selectors']==c['selectors'] and len(set(c['selectors']))==61
 assert row['binary_sha256']==r['test_binaries'][c['backend']]['sha256'] and row['environment_overrides']==c['environment_overrides']
 assert 'GPU_PHYSICS_TEST_TRACE' not in c['environment_overrides']
 assert row['actual_NVIDIA_Vulkan'] and 'NVIDIA GeForce RTX 4070 SUPER' in (B/'raw'/c['name']/'stderr.log').read_text()
 assert row['stdout_sha256']==sha(B/'raw'/c['name']/'stdout.log') and row['stderr_sha256']==sha(B/'raw'/c['name']/'stderr.log')
 actual=re.findall(r'^test (\S+) \.\.\. (ok|FAILED)$',(B/'raw'/c['name']/'stdout.log').read_text(),re.M);assert [list(v) for v in actual]==row['individual_results'] and {n for n,v in actual}==set(c['selectors'])
 assert row['harness_result']==(['FAILED','60','1','0'] if c['ordering']==0 else ['ok','61','0','0'])
 assert row['exit']==(101 if c['ordering']==0 else 0) and row['pass']==(c['ordering']==1)
 for n,result in actual:
  if c['ordering']==0 and n==name:assert result=='FAILED';failed+=1
  else:assert result=='ok';applicable+=1
assert len(r['results'])==4 and applicable==a['applicable_selected_checks']==242 and failed==len(a['retained_failed_observations'])==2
assert a['original_campaign_all_assertions_pass'] is False and a['production_shader_retained'] is False and a['pre_candidate_contract_sha256']==sha(B/'raw/pre-candidate-contract.json')
print('PASS evidence audit: 21 indexed raw files; 244 original checks retained,242 applicable passes/two known inapplicable mode0 failures. No assertion/protocol/result modified or repeated. Shader restored; scene review/full release gates OPEN.')
