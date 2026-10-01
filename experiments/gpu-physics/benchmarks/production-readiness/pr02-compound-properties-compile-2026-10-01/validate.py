#!/usr/bin/env python3
from pathlib import Path
import json,hashlib,tarfile
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();I=json.loads((B/'raw-index.json').read_text())
for n,h in I.items():assert sha(B/n)==h,n
R=json.loads((B/'raw/rust-driver-receipt.json').read_text());C=json.loads((B/'raw/candidate-inputs.json').read_text());rejected=(B/'raw/protocol-before-change.json').exists();P=json.loads((B/'raw'/('protocol-before-change.json'if rejected else'protocol-before-builds.json')).read_text());assert R['protocol_sha256']==C['protocol_sha256']==sha(B/'raw'/('protocol-before-change.json'if rejected else'protocol-before-builds.json'));assert R['candidate_inputs_sha256']==sha(B/'raw/candidate-inputs.json')
for n,h in C['production_candidate_sha256'].items():assert sha(B/'raw/candidate'/n)==h
for n,h in P['production_files_before'if rejected else'baseline_production'].items():assert sha(B/'raw/baseline'/n)==h
if rejected:
 assert R['status']=='stopped'and len(R['builds'])==1;assert R['restoration']==P['production_files_before'];backends=['ordinary']
else:assert R['status']=='compiled'and [x['backend']for x in R['builds']]==['ordinary','native']and all(x['exit']==0 for x in R['builds']);backends=['ordinary','native']
for b in backends:
 D=B/'raw/candidate-complete-inputs';E=json.loads((D/f'{b}-build.json').read_text());assert [x['exit']for x in E['commands']]==([0,101]if rejected else[0,0]);assert E['status']==('failed'if rejected else'built')
 for j,x in enumerate(E['commands'],1):assert sha(D/f'{b}-build-{j}.log')==x['log_sha256']
 if rejected:assert 'error[E0061]'in(D/'ordinary-build-2.log').read_text()
 else:assert E['engine_sources']==E['engine_sources_after']and len(E['binary_sha256'])==len(E['test_binary_sha256'])==64
 with tarfile.open(D/f'{b}-compiled-sources.tar.gz')as T:
  for n,h in E['engine_sources'].items():assert hashlib.sha256(T.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h,n
 for n,h in C['production_candidate_sha256'].items():assert E['engine_sources'][n]==h
if not rejected:
 old=B.parent/'pr02-compound-properties-fix-2026-10-01';r=json.loads((old/'raw/candidate-inputs.json').read_text());assert r['production_candidate_sha256']==P['rejected_candidate']
 for n in C['production_candidate_sha256']:
  a=(old/'raw/candidate'/n).read_text();b=(B/'raw/candidate'/n).read_text()
  if n=='src/api/world.rs':a=a.replace('let world = b3_create_world(&crate::api::b3_default_world_def());','let gpu = pollster::block_on(GpuDevice::new(None)).expect("NVIDIA GPU");\n        let world = b3_create_world(gpu, &crate::api::b3_default_world_def());')
  assert a==b,n
print(f'Validated {len(I)} portable build/source files; '+('E0061 rejection/restoration retained, zero validation trials.'if rejected else'four successful compiles, no API acceptance.'))
