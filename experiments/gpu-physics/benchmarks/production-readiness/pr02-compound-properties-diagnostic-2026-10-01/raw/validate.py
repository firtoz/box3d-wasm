#!/usr/bin/env python3
"""Offline closed diagnostic verification. Never launches engine processes."""
from pathlib import Path
import json,hashlib,tarfile
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();idx=json.loads((B/'raw-index.json').read_text())
for n,h in idx.items():assert sha(B/n)==h,n
P=json.loads((B/'protocol.json').read_text());R=json.loads((B/'raw/receipt.json').read_text());assert R['status']=='diagnosed';assert sha(B/'protocol.json')==R['protocol_sha256'];assert (B/'protocol.json').read_bytes()==(B/'raw/protocol.json').read_bytes();assert R['sources_before']==R['sources_after'];assert R['sources_before'][P['fixture']]==P['fixture_sha256']
assert P['budget']=={'CPU_controls':1,'GPU_diagnostics':4,'production_candidates':0,'timing_runs':0,'retries':0};assert [x['configuration']for x in R['runs']]==P['order'];assert len(R['builds'])==9 and all(x['exit']==0 for x in R['builds'])
for name,obs,bad in [('CPU',928,0),('ordinary-gpu',1016,864),('native-gpu',1016,864),('ordinary-both',1016,832),('native-both',1016,832)]:
 d=B/'raw'/name;run=next(x for x in R['runs']if x['configuration']==name);rows=[json.loads(l)for l in (d/'stdout.jsonl').read_text().splitlines()];summary=rows.pop();assert summary==run['summary']=={'summary':True,'observations':obs,'mismatches':bad};assert len(rows)==obs and sum(not x['match']for x in rows)==bad;assert run['exit']==0
 for n in ['stdout.jsonl','stderr.log']:assert sha(d/n)==run['stdout_sha256'if n=='stdout.jsonl'else 'stderr_sha256']
 build=next(x for x in R['builds']if x.get('configuration')==name and 'binary_sha256'in x);assert build['binary_sha256']==run['binary_sha256'];assert '-ffp-contract=off'in build['command'] and '-DNDEBUG'not in build['command']
 if name!='CPU':assert 'NVIDIA GeForce RTX 4070 SUPER'in (d/'stderr.log').read_text();assert run['environment']['GPU_PHYSICS_BACKEND']=='vulkan'and run['environment']['GPU_PHYSICS_LIVE_CONTACT_ORDER']=='1'
with tarfile.open(B/'raw/fixture-source-inputs.tar.gz')as T:
 for n,h in R['sources_before'].items():assert hashlib.sha256(T.extractfile(n.replace('../../box3d/','box3d/')).read()).hexdigest()==h,n
for backend in ['ordinary','native']:
 d=B/'raw'/f'{backend}-gpu';A=json.loads((d/'adapter-receipt.json').read_text());arc=d/'compiled-adapter-inputs.tar.gz';assert A['sources_before']==A['sources_after']==R['sources_before'];assert sha(arc)==A['source_archive_sha256']
 with tarfile.open(arc)as T:
  for u in A['compiled_units']:
   f=u['file'];token=f'/{backend}-gpu/cmake/';n='generated/'+f.split(token,1)[1]if token in f else 'box3d/'+f.split('/box3d/',1)[1]if '/box3d/'in f and '/experiments/'not in f else f.split('/experiments/gpu-physics/',1)[1];assert hashlib.sha256(T.extractfile(n).read()).hexdigest()==u['source_sha256'],n
 proof=B.parent/'pr02-compound-aabb-compile-2026-10-01/raw/candidate-complete-inputs'/f'{backend}-build.json';assert sha(proof)==P['engine_proof'][backend]['receipt_sha256'];E=json.loads(proof.read_text());assert E['engine_sources']==E['engine_sources_after']and E['binary_sha256']==P['engine_proof'][backend]['library_sha256']
 proof=B.parent/'pr02-compound-ownership-after-bounds-2026-10-01/raw'/f'{backend}-both/adapter-receipt.json';assert sha(proof)==P['combined_proof'][backend]['receipt_sha256'];E=json.loads(proof.read_text());assert E['sources_before']==E['sources_after']
print(f'Validated {len(idx)} portable raw files and five first diagnostic processes; GPU mismatches retained, no API acceptance.')
