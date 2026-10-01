from pathlib import Path
import hashlib,json,shutil
r=Path.cwd();a=r/'artifacts/production-readiness/pr02-compound-ownership-after-bounds';o=r/'benchmarks/production-readiness/pr02-compound-ownership-after-bounds-2026-10-01'
assert json.loads((a/'receipt.json').read_text())['status'] in ['pass','stopped']
o.mkdir(exist_ok=False);raw=o/'raw';raw.mkdir()
for f in sorted(a.rglob('*')):
 if not f.is_file() or 'cmake' in f.parts or '__pycache__'in f.parts or 'native-viewer'in f.parts or f.name=='fixture':continue
 d=raw/f.relative_to(a);d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,d)
shutil.copyfile(a/'protocol.json',o/'protocol.json');shutil.copyfile(a/'validate.py',o/'validate.py')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(o/'raw-index.json').write_text(json.dumps({str(f.relative_to(o)):sha(f)for f in sorted(raw.rglob('*'))if f.is_file()},indent=2)+'\n')
print('Published correctness raw data',len(list(raw.rglob('*'))))
