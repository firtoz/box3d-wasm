from pathlib import Path
import hashlib,json,shutil
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for campaign,status in [('pr02-compound-properties-fix','stopped'),('pr02-compound-properties-compile','compiled')]:
 A=Path('artifacts/production-readiness')/campaign;B=Path('benchmarks/production-readiness')/(campaign+'-2026-10-01');assert json.loads((A/'rust-driver-receipt.json').read_text())['status']==status;B.mkdir(exist_ok=False);raw=B/'raw';raw.mkdir()
 for f in sorted(A.rglob('*')):
  if not f.is_file()or'__pycache__'in f.parts or f.suffix=='.a'or f.name.endswith('-tests'):continue
  d=raw/f.relative_to(A);d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,d)
 (B/'raw-index.json').write_text(json.dumps({str(f.relative_to(B)):sha(f)for f in sorted(raw.rglob('*'))if f.is_file()},indent=2)+'\n')
 print('Published',campaign)
