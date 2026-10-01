#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,tarfile
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();I=json.loads((B/'raw-index.json').read_text())
for n,h in I.items():assert sha(B/n)==h,n
P=json.loads((B/'protocol.json').read_text());R=json.loads((B/'raw/receipt.json').read_text());assert R['protocol_sha256']==sha(B/'protocol.json');assert (B/'protocol.json').read_bytes()==(B/'raw/protocol.json').read_bytes();assert R['status']=='stopped'and len(R['runs'])==10 and R['restoration']==P['baseline_production'];assert R['unlaunched']==P['cases'][10:];assert len(R['builds'])==30 and all(x['exit']==0 for x in R['builds']);assert R['sources_before']==R['sources_after']
for b,e in P['engine_proof'].items():
 proof=B.parent/'pr02-compound-properties-compile-2026-10-01/raw/candidate-complete-inputs'/f'{b}-build.json';assert sha(proof)==e['receipt_sha256'];E=json.loads(proof.read_text());assert E['engine_sources']==E['engine_sources_after']and E['binary_sha256']==e['library_sha256']and E['test_binary_sha256']==e['tests_sha256']
for cfg in P['configurations']:
 d=B/'raw'/cfg;A=json.loads((d/'adapter-receipt.json').read_text());arc=d/'compiled-adapter-inputs.tar.gz';assert A['sources_before']==A['sources_after']==R['sources_before'];assert sha(arc)==A['source_archive_sha256']
 with tarfile.open(arc)as T:
  for u in A['compiled_units']:
   f=u['file'];token=f'/{cfg}/cmake/';n='generated/'+f.split(token,1)[1]if token in f else 'box3d/'+f.split('/box3d/',1)[1]if '/box3d/'in f and '/experiments/'not in f else f.split('/experiments/gpu-physics/',1)[1];assert hashlib.sha256(T.extractfile(n).read()).hexdigest()==u['source_sha256'],n
for x in R['builds']:assert sha(B/'raw'/x['log_path'])==x['log_sha256']
for i,x in enumerate(R['runs']):
 assert {k:x[k]for k in ['configuration','fixture','arguments']}==P['cases'][i]
 for n in ['stdout','stderr']:assert sha(B/'raw'/x[n+'_path'])==x[n+'_sha256']
 if i<9:
  assert x['exit']==0;rows=[json.loads(l)for l in(B/'raw'/x['stdout_path']).read_text().splitlines()];summary=rows.pop();assert summary==x['summary']=={'summary':True,'observations':928 if i==0 else 1016,'mismatches':0};assert len(rows)==summary['observations']and all(r['match']for r in rows)
  build=next(y for y in R['builds']if y.get('configuration')==x['configuration']and y.get('fixture')==x['fixture']);assert build['binary_sha256']==x['binary_sha256']
 else:assert x['exit']==101 and 'require actual slot reuse'in(B/'raw'/x['stderr_path']).read_text()and '0 passed; 1 failed'in(B/'raw'/x['stdout_path']).read_text()
 if i:assert 'NVIDIA GeForce RTX 4070 SUPER'in(B/'raw'/x['stderr_path']).read_text()
assert sha(B/'raw/frozen-reproduction-inputs'/P['physical_validator'])==P['physical_validator_sha256'];assert sha(B/'raw/frozen-reproduction-inputs/scripts/native-samples-cache-env.sh')==P['runtime_environment_source_sha256']
print(f'Validated {len(I)} portable files,9 property passes +retained failed reuse assumption;23 cases unlaunched.')
