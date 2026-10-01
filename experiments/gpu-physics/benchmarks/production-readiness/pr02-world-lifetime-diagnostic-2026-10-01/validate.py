from pathlib import Path
import json,hashlib,tarfile
B=Path(__file__).resolve().parent;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();I=json.loads((B/'raw-index.json').read_text())
for n,h in I.items():assert sha(B/n)==h['sha256']and (B/n).stat().st_size==h['bytes'],n
R=json.loads((B/'raw/receipt.json').read_text());P=json.loads((B/'raw/protocol-before-runs.json').read_text());assert R['status']=='diagnosed';assert R['protocol_sha256']==sha(B/'raw/protocol-before-runs.json');assert sha(B/'raw/world_lifetime_diagnostic.cpp')==P['fixture_sha256'];assert P['budget']=={'fixture_compiles':5,'CPU_control':1,'GPU_diagnostics':4,'production_candidates':0,'timing_runs':0,'retries':0};assert [x['configuration']for x in R['builds']]==[x['configuration']for x in R['runs']]==P['order'];assert all(x['exit']==0 for x in R['builds'])
proof=B.parent/'pr02-controls-current-builds-2026-10-01/raw';root=json.loads((proof/'receipt.json').read_text());assert sha(proof/'receipt.json')==P['proof_receipt_sha256'];assert root['inputs_before']==root['inputs_after'];original={}
with tarfile.open(proof/'compiled-source-inputs.tar.gz')as t:
 for m in t.getmembers():
  if m.isfile():original[m.name]=hashlib.sha256(t.extractfile(m).read()).hexdigest()
fields={'distinct-world-id','old-stays-invalid','old-cannot-read-fresh-user-data','fresh-warm-start-preserved','fresh-stays-valid','live-stays-valid'}
for cell,build,run in zip(P['order'],R['builds'],R['runs']):
 folder=B/'raw'/cell;assert sha(folder/'build.log')==build['log_sha256'];assert run['binary_sha256']==build['binary_sha256'];assert run['exit']==0;assert sha(folder/'stdout.jsonl')==run['stdout_sha256'];assert sha(folder/'stderr.log')==run['stderr_sha256'];rows=[json.loads(line)for line in(folder/'stdout.jsonl').read_text().splitlines()];checks=[s for s in rows if 'match'in s];summary=rows[-1];assert summary==run['summary'];assert summary['observations']==len(checks)==(12 if cell=='cpu'else 13);assert summary['mismatches']==sum(not s['match']for s in checks)==(0 if cell=='cpu'else 6);assert all(s['match']==(s['got']==s['expected'])for s in checks)
 ids=next(s for s in rows if s.get('ids'));assert (ids['old']!=ids['fresh'])==(cell=='cpu');assert {s['field']for s in checks if not s['match']}==(set()if cell=='cpu'else fields)
 if cell!='cpu':assert ids['old']==ids['fresh']==[1,1]and ids['other']==[2,1];assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan driver=NVIDIA'in(folder/'stderr.log').read_text()
 v=json.loads((proof/cell/'receipt.json').read_text());assert sha(proof/cell/'receipt.json')==P['viewer_receipts'][cell]==R['proof'][cell]['viewer_receipt_sha256'];generated={}
 with tarfile.open(proof/cell/'generated-inputs.tar.gz')as t:
  for m in t.getmembers():
   if m.isfile():generated[m.name]=hashlib.sha256(t.extractfile(m).read()).hexdigest()
 for u in R['proof'][cell]['applicable_compiled_units']:
  assert u in v['compiled_units'];file=u['file']
  if f'/pr02-controls-current-builds/{cell}/cmake/'in file:actual=generated[file.split(f'/pr02-controls-current-builds/{cell}/cmake/',1)[1]]
  elif '/box3d-wasm/box3d/'in file:actual=original['box3d/'+file.split('/box3d-wasm/box3d/',1)[1]]
  else:actual=original[file.split('/experiments/gpu-physics/',1)[1]]
  assert actual==u['source_sha256']
 for file,h in R['proof'][cell]['linked_inputs'].items():
  if file in v['linked_archives']:assert v['linked_archives'][file]==h
  elif file.endswith('libbox3d_cpu.a'):
   donor=json.loads((proof/(cell.split('-')[0]+'-both')/'receipt.json').read_text());assert donor['linked_archives'][file]==h
  else:assert h==root['libraries'][cell.split('-')[0]]['sha256']
print(len(I),'raw files verified; CPU12/0, each GPU13/6; root lifetime failure on all4 cells, no readiness acceptance')
