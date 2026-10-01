#!/usr/bin/env python3
"""Offline audit of rejected build, compile repair and first API validation."""
from pathlib import Path
import hashlib,json,tarfile
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def load(root,n):return json.loads((root/n).read_text())
def index(root):
 entries=load(root,'raw-index.json')['entries'];assert len({e['path'] for e in entries})==len(entries)
 for e in entries:assert sha(root/e['path'])==e['sha256'] and (root/e['path']).stat().st_size==e['bytes']
 return len(entries)
def archive(root,path,expected,rename=lambda n:n):
 with tarfile.open(root/path) as a:
  for n,h in expected.items():assert hashlib.sha256(a.extractfile(rename(n)).read()).hexdigest()==h,n

def main():
 total=index(R);failed=R.parent/'pr02-compound-aabb-fix-2026-10-01';compiled=R.parent/'pr02-compound-aabb-compile-2026-10-01';total+=index(failed)+index(compiled)
 p=load(failed,'protocol.json');stop=load(failed,'raw/stop.json')
 assert (failed/'protocol.json').read_bytes()==(failed/'raw/protocol-before-runs.json').read_bytes()
 assert p['initial_protocol_sha256']==sha(failed/'raw/protocol-initial-before-build.json')
 assert stop['status']=='rejected-at-build' and stop['validation_processes']==stop['timing_runs']==0
 assert 'sequencing_deviation' in stop
 for phase in ['baseline','rejected']:archive(failed,f'raw/{phase}-production-inputs.tar.gz',p[('baseline' if phase=='baseline' else 'candidate')+'_production'])
 for b in ['ordinary','native']:
  r=load(failed,f'raw/candidate-complete-inputs/{b}-build.json');assert r['status']=='failed' and len(r['commands'])==1 and r['commands'][0]['exit']==101
  assert sha(failed/f'raw/candidate-complete-inputs/{b}-build-1.log')==r['commands'][0]['log_sha256']
  assert 'error[E0282]' in (failed/f'raw/candidate-complete-inputs/{b}-build-1.log').read_text()
  archive(failed,f'raw/candidate-complete-inputs/{b}-compiled-sources.tar.gz',r['engine_sources'],lambda n:n.replace('../../box3d/','box3d/'))
 p=load(compiled,'protocol.json');assert load(compiled,'raw/driver-receipt.json')['status']=='compiled'
 assert (compiled/'protocol.json').read_bytes()==(compiled/'raw/protocol-before-builds.json').read_bytes()
 assert p['budget']['GPU_processes']==p['budget']['timing_runs']==0
 archive(compiled,'raw/baseline-production-inputs.tar.gz',p['baseline_production']);archive(compiled,'raw/candidate-production-inputs.tar.gz',p['candidate_production'])
 for b in ['ordinary','native']:
  r=load(compiled,f'raw/candidate-complete-inputs/{b}-build.json');assert r['status']=='built' and len(r['commands'])==2
  assert r['engine_sources']==r['engine_sources_after']
  for i,c in enumerate(r['commands'],1):assert c['exit']==0 and sha(compiled/f'raw/candidate-complete-inputs/{b}-build-{i}.log')==c['log_sha256']
  archive(compiled,f'raw/candidate-complete-inputs/{b}-compiled-sources.tar.gz',r['engine_sources'],lambda n:n.replace('../../box3d/','box3d/'))
 p=load(R,'protocol.json');r=load(R,'raw/receipt.json');assert r['status']=='pass' and 'running' not in r
 assert (R/'protocol.json').read_bytes()==(R/'raw/protocol-before-runs.json').read_bytes()
 assert r['protocol_sha256']==sha(R/'protocol.json')
 assert r['C_sources_before']==r['C_sources_after'] and r['C_sources_before']['c_abi/compound_aabb_contract.cpp']==p['fixture_sha256']
 archive(R,'raw/C-fixture-inputs.tar.gz',r['C_sources_before'],lambda n:n.replace('../../box3d/','box3d/'))
 assert len(r['builds'])==10 and all(b['exit']==0 for b in r['builds'])
 for cfg in ['ordinary-both','native-both']:
  a=load(R,f'raw/{cfg}/adapter-receipt.json');assert a['C_sources_before']==a['C_sources_after']==r['C_sources_before']
  archive(R,f'raw/{cfg}/compiled-adapter-inputs.tar.gz',a['generated_sources'],lambda n:'generated/'+n)
  backend=cfg.split('-')[0];proof=p['engine_proof'][backend];prior=load(compiled,f'raw/candidate-complete-inputs/{backend}-build.json')
  assert sha(R/proof['receipt_portable'])==proof['receipt_sha256']==a['engine_receipt_sha256']
  assert proof['library_sha256']==prior['binary_sha256'] and proof['test_sha256']==prior['test_binary_sha256']
  t=load(R,f'raw/{cfg}/translation-proof.json');assert len(t['compiled_units'])==108
  assert sha(R/f'raw/{cfg}/compiled-translation-inputs.tar.gz')==t['archive_sha256']
  archive(R,f'raw/{cfg}/compiled-translation-inputs.tar.gz',t['generated_sources'],lambda n:'generated/'+n)
  with tarfile.open(R/f'raw/{cfg}/compiled-translation-inputs.tar.gz') as source:
   for unit in t['compiled_units']:
    n=unit['source'];part=('generated/'+n.split('/cmake/',1)[1] if '/cmake/' in n else 'source/'+n.split('/work/2026/box3d-wasm/',1)[1])
    assert hashlib.sha256(source.extractfile(part).read()).hexdigest()==unit['source_sha256']
  for build in r['builds']:
   if build.get('configuration')==cfg:
    assert build['linked_inputs']==a['linked_inputs'];assert sha(R/f"raw/{cfg}/{build['fixture']}/build.log")==build['log_sha256']
 cases=[('C',b,'compound_aabb_contract') for b in ['ordinary','native','native','ordinary']]+[('C',b,n) for n in p['C_regressions'] for b in ['ordinary','native']]+[('Rust',b,n) for n in p['Rust_suites'] for b in ['ordinary','native']]
 assert [(x['kind'],x['backend'],x['selector']) for x in r['runs']]==cases
 for x in r['runs']:
  assert x['exit']==0
  for stream in ['stdout','stderr']:assert sha(R/'raw'/x[stream+'_path'])==x[stream+'_sha256']
  stdout=(R/'raw'/x['stdout_path']).read_text()
  if x['kind']=='Rust':
   assert x['binary_sha256']==p['engine_proof'][x['backend']]['test_sha256']
   assert f"{x['expected_test_count']} passed; 0 failed; 0 ignored" in stdout
  else:
   assert x['binary_sha256']==next(b['binary_sha256'] for b in r['builds'] if b.get('configuration')==x['backend']+'-both' and b.get('fixture')==x['selector'])
   assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in (R/'raw'/x['stderr_path']).read_text()
   if x['selector']=='compound_aabb_contract':
    assert stdout.count('maxError=')==320 and 'compound AABB contract:' in stdout
    assert all(float(line.rsplit('=',1)[1])<=1e-5 for line in stdout.splitlines() if 'maxError=' in line)
 print(f'Validated {total} portable raw files:2 retained compiler failures,4 successful compile commands,12 passing first API processes. No timing or final readiness acceptance.')
if __name__=='__main__':main()
