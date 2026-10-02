from pathlib import Path
import json,hashlib
R=Path.cwd();D=R/'benchmarks/production-readiness/pr09-floor-2026-10-02';A=R/'artifacts/production-readiness/pr09-floor';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();B=A/'build';assert json.loads((B/'receipt.json').read_text())['status']=='built'
p=json.loads((A/'before/protocol.json').read_text());p['artifact_directory']='artifacts/production-readiness/pr09-floor/after';p['source_files']['scripts/inject-sokol-capacity.py']=sha(R/'scripts/inject-sokol-capacity.py');p['source_files']['c_abi/growable_slots.h']=sha(R/'c_abi/growable_slots.h')
for k,v in p['gpu_viewers'].items():
 d=json.loads((B/k/'receipt.json').read_text());v.update(binary=str((B/k/'viewer').relative_to(R)),binary_sha256=d['executable_sha256'],receipt=str((B/k/'receipt.json').relative_to(R)),receipt_sha256=sha(B/k/'receipt.json'),label='2026-10-02-pr09-floor-after-'+k)
p['cpu_reference']='Reuse the five exact CPU-first before clips: real Box3D physics/geometry/scene/default objects unchanged. Newly rebuilt CPU adapter viewer has a build receipt but is not run; no second CPU capture consumption.'
(A/'after').mkdir();f=A/'after/protocol.json';f.write_text(json.dumps(p,indent=2)+'\n');before=json.loads((A/'before/receipt.json').read_text());state={'status':'cpu-complete','protocol_sha256':sha(f),'results':[r for r in before['results'] if r['kind']=='cpu'],'cpu_reference_receipt_sha256':sha(A/'before/receipt.json')};(A/'after/receipt.json').write_text(json.dumps(state,indent=2)+'\n');print('after frozen',sha(f))
