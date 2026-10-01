#!/usr/bin/env python3
"""Validate complete API diagnosis offline; no physics is launched."""
from pathlib import Path
import hashlib,json,tarfile,re
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
load=lambda p:json.loads((R/p).read_text())
def main():
 entries=load('raw-index.json')['entries']
 assert len({e['path'] for e in entries})==len(entries)
 for e in entries:assert sha(R/e['path'])==e['sha256'] and (R/e['path']).stat().st_size==e['bytes']
 p=load('protocol.json');r=load('raw/receipt.json')
 assert p['budget']=={'GPU_diagnostic_processes':2,'production_candidates':0,'timing_runs':0}
 assert (R/'protocol.json').read_bytes()==(R/'raw/protocol-before-runs.json').read_bytes()
 assert r['protocol_sha256']==sha(R/'protocol.json') and r['status']=='diagnosis-complete' and 'running' not in r
 assert r['sources_before']==r['sources_after']
 assert r['sources_before']['c_abi/compound_bounds_diagnostic.cpp']==p['fixture_sha256']
 assert r['sources_before']['c_abi/both_dual.c']==p['baseline_dual_source_sha256']
 with tarfile.open(R/'raw/fixture-inputs.tar.gz') as a:
  for n,h in r['sources_before'].items():assert hashlib.sha256(a.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h
 assert len(r['builds'])==len(r['runs'])==2
 observed=[]
 for b,x,cfg in zip(r['builds'],r['runs'],p['order']):
  assert b['configuration']==x['configuration']==cfg and b['exit']==x['exit']==0
  assert b['binary_sha256']==x['binary_sha256']
  assert sha(R/'raw'/cfg/'build.log')==b['log_sha256']
  proof=p['source_proof'][cfg]
  for name,h in b['linked_inputs'].items():
   if '/pr02-compound-fix/' in name:assert h==proof['baseline_archive_sha256']
   else:assert proof['linked_inputs'][name]==h
  for field in ['engine','adapter']:assert sha(R/proof[field+'_receipt_portable'])==proof[field+'_receipt_sha256']
  for stream in ['stdout','stderr']:assert sha(R/'raw'/cfg/(stream+'.log'))==x[stream+'_sha256']
  assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in (R/'raw'/cfg/'stderr.log').read_text()
  rows=[]
  for line in (R/'raw'/cfg/'stdout.log').read_text().splitlines():
   m=re.fullmatch(r'(sphere|capsule|hull|mesh) rotated=(\d) lane=(\d) CPU=(\S+) independentCPU=(\S+) GPUpublic=(\S+) GPUnative=(\S+) publicError=(\S+) nativeError=(\S+)',line)
   if m:rows.append(dict(zip(['configuration','kind','rotated','lane','CPU','independentCPU','GPUpublic','GPUnative','publicError','nativeError'],[cfg]+list(m.groups()))))
  assert len(rows)==48
  assert {(q['kind'],q['rotated'],q['lane']) for q in rows}=={(k,str(o),str(i)) for k in ['sphere','capsule','hull','mesh'] for o in range(2) for i in range(6)}
  assert all(abs(float(q['CPU'])-float(q['independentCPU']))<=1e-5 and float(q['GPUpublic'])==0 for q in rows)
  assert max(float(q['nativeError']) for q in rows if q['kind']=='sphere' and q['rotated']=='1')>.1
  observed+=rows
 assert observed==load('observations.json')['rows']
 print(f'Validated {len(entries)} raw files and96 lane observations:2 diagnostic processes, confirmed public bounds defects, no acceptance/timing/candidate credit.')
if __name__=='__main__':main()
