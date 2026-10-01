#!/usr/bin/env python3
"""Offline audit of a closed ownership protocol; launches no physics."""
from pathlib import Path
import hashlib,json,tarfile
B=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
idx=json.loads((B/'raw-index.json').read_text())
for n,h in idx.items():assert sha(B/n)==h,n
p=json.loads((B/'protocol.json').read_text());r=json.loads((B/'raw/receipt.json').read_text());c=json.loads((B/'raw/candidate-inputs.json').read_text())
assert (B/'protocol.json').read_bytes()==(B/'raw/protocol.json').read_bytes()
assert r['protocol_sha256']==c['protocol_sha256']==sha(B/'protocol.json')
assert r['candidate_inputs_sha256']==sha(B/'raw/candidate-inputs.json')
assert p['budget']=={'production_candidates':1,'baseline_processes':0,'ownership_processes':4,'regression_processes':10,'Rust_builds_or_trials':0,'timing_runs':0,'retries':0}
assert sha(B/'raw/baseline-both_dual.c')==p['baseline_source_sha256']
assert sha(B/'raw/candidate-both_dual.c')==c['candidate_source_sha256']==r['sources_before']['c_abi/both_dual.c']
assert r['sources_before']['c_abi/both_compound_ownership_test.cpp']==p['ownership_fixture_sha256']
for n,h in p['regressions'].items():assert r['sources_before'][f'c_abi/{n}.cpp']==h
assert r['sources_before']==r['sources_after']
for backend in ['ordinary','native']:
 d=B/'raw'/f'{backend}-both';a=json.loads((d/'adapter-receipt.json').read_text());arc=d/'compiled-adapter-inputs.tar.gz'
 assert a['sources_before']==a['sources_after']==r['sources_before'];assert a['engine_proof']==p['engine_proof'][backend]
 assert sha(arc)==a['source_archive_sha256']
 with tarfile.open(arc)as t:
  for n,h in a['sources_before'].items():assert hashlib.sha256(t.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h,n
  for u in a['compiled_units']:
   # CMake inventories a shared cached NFD object too; it is not linked into any C fixture.
   if '.fetchcontent-cache/nfd-build/'in u['object']:
    assert 'nfd'not in ' '.join(a['linked_inputs']);continue
   f=u['file'];n='generated/'+f.split(f'/{backend}-both/cmake/',1)[1]if f'/{backend}-both/cmake/'in f else 'box3d/'+f.split('/box3d/',1)[1]if '/box3d/'in f and '/experiments/'not in f else f.split('/experiments/gpu-physics/',1)[1]
   assert hashlib.sha256(t.extractfile(n).read()).hexdigest()==u['source_sha256'],n
 # Rust provenance is in the already published compile-only repair, exact before/after inputs.
 e=B.parent/'pr02-compound-aabb-compile-2026-10-01/raw/candidate-complete-inputs'/f'{backend}-build.json'
 assert sha(e)==p['engine_proof'][backend]['receipt_sha256'];proof=json.loads(e.read_text());assert proof['engine_sources']==proof['engine_sources_after']
 assert proof['binary_sha256']==p['engine_proof'][backend]['library_sha256']
expected=[(b,'both_compound_ownership_test')for b in ['ordinary','native','native','ordinary']]+[(b,n)for n in p['regressions']for b in ['ordinary','native']]
assert [(x['backend'],x['fixture'])for x in r['runs']]==expected
markers={'both_compound_ownership_test':'combined compound ownership:','native_diagnostic_populations':'native diagnostic populations:','api_settings_test':'C API settings: pass','both_shape_replacement_test':'shape replacement:','both_substep_forces_test':'substep forces:','compound_aabb_contract':'compound AABB contract:'}
for x in r['runs']:
 assert x['exit']==0
 assert sha(B/'raw'/x['stdout_path'])==x['stdout_sha256'];assert sha(B/'raw'/x['stderr_path'])==x['stderr_sha256']
 assert markers[x['fixture']]in (B/'raw'/x['stdout_path']).read_text()
 assert 'NVIDIA GeForce RTX 4070 SUPER' in (B/'raw'/x['stderr_path']).read_text()
 assert x['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='1'and x['environment']['GPU_PHYSICS_BACKEND']=='vulkan'
 b=next(b for b in r['builds']if b.get('configuration')==x['backend']+'-both'and b.get('fixture')==x['fixture'])
 assert b['exit']==0 and b['binary_sha256']==x['binary_sha256'];assert '-ffp-contract=off'in b['command']and '-DNDEBUG'not in b['command']
assert r['status']=='pass'and len(r['runs'])==14
print(f'Validated {len(idx)} portable raw files,14 first processes, original assertions/bounds; no timing or release qualification.')
