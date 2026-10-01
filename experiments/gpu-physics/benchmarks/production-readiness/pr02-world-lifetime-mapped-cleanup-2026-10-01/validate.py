from pathlib import Path
import json,hashlib
B=Path(__file__).resolve().parent;H=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();J=lambda p:json.loads(p.read_text());index=J(B/'raw-index.json')
for path,r in index.items():p=B/path;assert p.stat().st_size==r['bytes']and H(p)==r['sha256']
r=J(B/'raw/receipt.json');P=J(B/'raw/protocol-before-builds.json');assert r['status']=='passed'and len(r['builds'])==len(r['runs'])==2;assert H(B/'raw/protocol-before-builds.json')==r['protocol_sha256'];assert P['budget']['timing']==P['budget']['retries']==P['budget']['Rust_C_adapter_rebuilds']==0
prior=B.parent/'pr02-world-lifetime-repair-2026-10-01/raw/pr02-world-lifetime-C-applicability/receipt.json';assert H(prior)==r['prerequisite_receipt_sha256'];source=J(prior)
for build,result in zip(r['builds'],r['runs']):
 cell=build['configuration'];assert cell==result['configuration']and build['exit']==result['exit']==0;assert build['linked_inputs']==source['proof'][cell]['linked_inputs'];assert build['binary_sha256']==result['binary_sha256'];assert result['summary']=={'summary':True,'observations':21,'mismatches':0};rows=[json.loads(l)for l in(B/'raw'/cell/'stdout.jsonl').read_text().splitlines()];checks=[x for x in rows if'match'in x];assert len(checks)==21 and all(x['match']for x in checks);assert result['environment']['GPU_PHYSICS_ADAPTER']=='nvidia'and result['environment']['GPU_PHYSICS_BACKEND']=='vulkan'
assert H(B/'raw/fixture.cpp')==r['fixture']['sha256'];assert H(B/'raw/ordinary-both/stdout.jsonl')==r['runs'][0]['stdout_sha256'];assert H(B/'raw/native-both/stdout.jsonl')==r['runs'][1]['stdout_sha256']
print(len(index),'portable raw files verified; two first processes/42 observations pass. No device-loss, viewer, capture, release or performance acceptance.')
