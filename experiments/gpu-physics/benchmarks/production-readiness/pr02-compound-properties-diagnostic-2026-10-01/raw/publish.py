from pathlib import Path
import json,hashlib,shutil
A=Path('artifacts/production-readiness/pr02-compound-properties-diagnostic');B=Path('benchmarks/production-readiness/pr02-compound-properties-diagnostic-2026-10-01');assert json.loads((A/'receipt.json').read_text())['status']=='diagnosed';B.mkdir(exist_ok=False);raw=B/'raw';raw.mkdir()
for f in sorted(A.rglob('*')):
 if not f.is_file() or 'cmake'in f.parts or '__pycache__'in f.parts or f.name=='fixture':continue
 d=raw/f.relative_to(A);d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,d)
for n in ['protocol.json','validate.py']:shutil.copyfile(A/n,B/n)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(B/'raw-index.json').write_text(json.dumps({str(f.relative_to(B)):sha(f)for f in sorted(raw.rglob('*'))if f.is_file()},indent=2)+'\n')
print('Published diagnostic',B)
