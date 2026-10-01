#!/usr/bin/env python3
"""Offline handle-fixture correction and remaining original-limit checks."""
from pathlib import Path
import hashlib,json,tarfile,math
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();I=json.loads((B/'raw-index.json').read_text())
for n,h in I.items():assert sha(B/n)==h,n
P=json.loads((B/'protocol.json').read_text());R=json.loads((B/'raw/receipt.json').read_text());C=json.loads((B/'raw/candidate-inputs.json').read_text());assert R['status']=='pass'and len(R['runs'])==24 and len(R['builds'])==2;assert R['protocol_sha256']==C['protocol_sha256']==sha(B/'protocol.json');assert R['candidate_inputs_sha256']==sha(B/'raw/candidate-inputs.json');assert (B/'protocol.json').read_bytes()==(B/'raw/protocol-before-correction.json').read_bytes()
OLD=B.parent/'pr02-compound-properties-validation-2026-10-01';COMP=B.parent/'pr02-compound-properties-compile-2026-10-01';assert sha(OLD/'protocol.json')==P['prior_validation_protocol_sha256'];assert sha(OLD/'raw/receipt.json')==P['prior_validation_receipt_sha256'];old=json.loads((OLD/'raw/receipt.json').read_text());assert old['status']=='stopped'and len(old['runs'])==10;assert P['remaining_C_cases']==json.loads((OLD/'protocol.json').read_text())['cases'][11:];assert len(P['remaining_C_cases'])==22
marker='#[cfg(test)]\nmod compound_property_contract {'
for n,h in C['production_candidate_sha256'].items():
 assert sha(B/'raw/candidate'/n)==h;assert sha(B/'raw/baseline'/n)==P['baseline_production'][n]
 a=(COMP/'raw/candidate'/n).read_text();b=(B/'raw/candidate'/n).read_text()
 if n=='src/api/world.rs':assert a.count(marker)==b.count(marker)==1 and a.split(marker)[0]==b.split(marker)[0];assert 'owned_table_and_invalid_handles_preserve_public_properties'in b and 'require actual slot reuse'not in b and 'let wrong_generation = ShapeId'in b
 else:assert a==b
for x in R['builds']:
 b=x['backend'];assert x['exit']==0;assert sha(B/'raw'/x['log_path'])==x['log_sha256'];assert x['sources_before']==x['sources_after'];arc=B/'raw'/b/'test-compiled-sources.tar.gz';assert sha(arc)==x['source_archive_sha256']
 with tarfile.open(arc)as T:
  for n,h in x['sources_before'].items():assert hashlib.sha256(T.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h,n
 e=json.loads((COMP/'raw/candidate-complete-inputs'/f'{b}-build.json').read_text());assert sha(COMP/'raw/candidate-complete-inputs'/f'{b}-build.json')==P['engine_proof'][b]['receipt_sha256'];assert e['binary_sha256']==P['engine_proof'][b]['library_sha256']
 for n,h in e['engine_sources'].items():
  if n=='src/api/world.rs':assert x['sources_before'][n]==C['production_candidate_sha256'][n]
  else:assert x['sources_before'][n]==h,n
cases=[{'configuration':b+'-rust','fixture':'compound_property_contract','arguments':['compound_property_contract','--test-threads=1','--nocapture']}for b in ['ordinary','native']]+P['remaining_C_cases'];assert len(cases)==len(R['runs'])
source=(OLD/'raw/run.py').read_text();mesh_source=source[source.index('def mesh_check('):source.index('\ntry:')];ns={'math':math};exec(mesh_source,ns);cpu_modes={};markers={'native_diagnostic_populations':'native diagnostic populations:','api_settings_test':'C API settings: pass','both_compound_ownership_test':'combined compound ownership:','compound_aabb_contract':'compound AABB contract:'}
for i,x in enumerate(R['runs']):
 assert {k:x[k]for k in ['configuration','fixture','arguments']}==cases[i]and x['exit']==0
 for n in ['stdout','stderr']:assert sha(B/'raw'/x[n+'_path'])==x[n+'_sha256']
 cfg=x['configuration'];name=x['fixture'];stdout=B/'raw'/x['stdout_path'];text=stdout.read_text()
 if i<2:
  assert '1 passed; 0 failed; 0 ignored;'in text and 'owned_table_and_invalid_handles_preserve_public_properties ... ok'in text
  e=next(y for y in R['builds']if y['backend']==cfg.split('-')[0]);assert x['binary_sha256']==e['test_binary_sha256']
 else:
  e=next(y for y in old['builds']if y.get('configuration')==cfg and y.get('fixture')==name);assert e['binary_sha256']==x['binary_sha256']
  if name=='compound_mesh_reference':
   mode=x['arguments'][0]
   if cfg=='CPU':cpu_modes[mode]=stdout
   assert ns['mesh_check'](cpu_modes[mode],stdout)==x['mesh_validation']
  else:assert markers[name]in text
 if cfg!='CPU':assert 'NVIDIA GeForce RTX 4070 SUPER'in (B/'raw'/x['stderr_path']).read_text();assert x['environment']['GPU_PHYSICS_BACKEND']=='vulkan'and x['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='1'
assert P['budget']['GPU_property_processes']==P['budget']['production_candidates']==P['budget']['timing_runs']==P['budget']['retries']==0
print(f'Validated {len(I)} portable files,2 corrected handle tests +22 first mesh/regression processes; no property reruns or allocator ABA claim.')
