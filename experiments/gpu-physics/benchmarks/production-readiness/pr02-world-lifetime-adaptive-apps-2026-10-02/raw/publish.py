from pathlib import Path
import hashlib,json,shutil
root=Path.cwd();src=root/'artifacts/production-readiness/pr02-world-lifetime-adaptive-apps';dest=root/'benchmarks/production-readiness/pr02-world-lifetime-adaptive-apps-2026-10-02';dest.mkdir(exist_ok=False);raw=dest/'raw';raw.mkdir();sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for p in src.rglob('*'):
 if not p.is_file() or 'pipelines'in p.parts or '__pycache__'in p.parts:continue
 if p.suffix=='.mp4':continue
 rel=p.relative_to(src);target=raw/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
index={str(p.relative_to(dest)):{'sha256':sha(p),'bytes':p.stat().st_size}for p in sorted(raw.rglob('*'))if p.is_file()};(dest/'raw-index.json').write_text(json.dumps(index,indent=2)+'\n');print(len(index),'portable raw files')
