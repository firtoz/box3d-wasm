#!/usr/bin/env python3
"""Validate retained failed campaign offline; never launch GPU physics."""
from pathlib import Path
import hashlib,json,tarfile
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads((R/p).read_text())

def main():
 entries=load('raw-index.json')['entries']
 assert len({e['path'] for e in entries})==len(entries)
 for e in entries:assert sha(R/e['path'])==e['sha256'] and (R/e['path']).stat().st_size==e['bytes']
 p=load('protocol.json');assert (R/'protocol.json').read_bytes()==(R/'raw/protocol-before-runs.json').read_bytes()
 assert p['budget']=={'baseline_C_processes':2,'production_candidates':1,'candidate_ownership_C_processes':4,'candidate_regression_C_processes':8,'timing_runs':0}
 for phase,count,status in [('baseline',2,'pass'),('candidate',10,'failed')]:
  r=load('raw/'+phase+'-receipt.json');assert r['status']==status and 'running' not in r
  assert r['protocol_sha256']==sha(R/'protocol.json') and len(r['builds'])==count
  assert r['sources_before']==r['sources_after']
  assert r['sources_before']['c_abi/both_compound_ownership_test.cpp']==p['fixture_sha256']
  with tarfile.open(R/('raw/'+phase+'-fixture-inputs.tar.gz')) as a:
   for n,h in r['sources_before'].items():assert hashlib.sha256(a.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h
  for b in r['builds']:
   assert b['exit']==0 and b['binary_sha256']
   assert sha(R/f"raw/{phase}/{b['configuration']}/{b['fixture']}/build.log")==b['log_sha256']
   proof=p['source_proof'][b['configuration']]
   for name,h in b['linked_inputs'].items():
    if phase=='candidate' and '/pr02-compound-fix/' in name:assert h==load('raw/candidate/'+b['configuration']+'/archive-build.json')['archive_sha256']
    elif phase=='baseline' and '/pr02-compound-fix/' in name:assert h==proof['baseline_archive_sha256']
    else:assert proof['linked_inputs'][name]==h
   for field in ['engine','adapter']:
    assert sha(R/proof[field+'_receipt_portable'])==proof[field+'_receipt_sha256']
  expected=[('ordinary-both',0),('native-both',0)] if phase=='baseline' else [('ordinary-both',-6)]
  assert [(x['configuration'],x['exit']) for x in r['runs']]==expected
  for x in r['runs']:
   d=R/f"raw/{phase}/{x['configuration']}/{x['fixture']}"
   for stream in ['stdout','stderr']:assert sha(d/f"{x['trial']}-{stream}.log")==x[stream+'_sha256']
   assert x['binary_sha256']==next(b['binary_sha256'] for b in r['builds'] if b['configuration']==x['configuration'] and b['fixture']==x['fixture'])
   assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in (d/f"{x['trial']}-stderr.log").read_text()
 for cfg in ['ordinary-both','native-both']:
  r=load('raw/candidate/'+cfg+'/archive-build.json');assert r['status']=='pass'
  assert r['sources_before']==r['sources_after']
  assert r['baseline_archive_sha256']==p['source_proof'][cfg]['baseline_archive_sha256']
  assert r['members_after']['both_dual.c.o']==r['object_sha256']
  for n in ['both_map.c.o','both_passthrough.c.o']:assert r['members_before'][n]==r['members_after'][n]
  assert sha(R/'raw/candidate-adapter-inputs.tar.gz')==r['source_archive_sha256']
  with tarfile.open(R/'raw/candidate-adapter-inputs.tar.gz') as a:
   for n,h in r['sources_before'].items():
    rel='box3d/'+n.split('/box3d/',1)[1] if '/box3d/include/' in n else n.split('/experiments/gpu-physics/',1)[1]
    assert hashlib.sha256(a.extractfile(rel).read()).hexdigest()==h
 assert sha(R/'raw/baseline-both_dual.c')==p['baseline_dual_source_sha256']
 failure=(R/'raw/candidate/ordinary-both/both_compound_ownership_test/1-stderr.log').read_text()
 assert 'compound bounds lane=0 GPU=-1.57911253 CPU=-1.6875484' in failure
 assert 'sphere create GPU bodies/shapes/joints=1/1/0 CPU=1/1/0' in (R/'raw/candidate/ordinary-both/both_compound_ownership_test/1-stdout.log').read_text()
 print(f'Validated {len(entries)} raw files, two baseline reproductions and one retained candidate failure; eleven candidate processes unlaunched, no timing acceptance.')

if __name__=='__main__':main()
