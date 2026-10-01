from pathlib import Path
import json,hashlib,tarfile,shutil,subprocess
R=Path.cwd();D=R/'benchmarks/production-readiness/pr02-world-lifetime-repair-2026-10-01';D.mkdir();raw=D/'raw';raw.mkdir();campaigns=['pr02-world-lifetime-fix','pr02-world-lifetime-artifact-repair','pr02-world-lifetime-rust-validation','pr02-world-lifetime-cleanup','pr02-world-lifetime-C-applicability','pr02-world-lifetime-CPU-contract'];a=R/'artifacts/production-readiness';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
# Preserve all non-binary raw outcomes, including the failed CPU process.
for name in campaigns:
 for p in(a/name).rglob('*'):
  rel=p.relative_to(a/name)
  if not p.is_file()or any(x in rel.parts for x in ['baseline','candidate-inputs','api-sources','pipelines']):continue
  if p.suffix not in ['.json','.jsonl','.log','.py','.cpp','.txt','.patch','.d','.toml']:continue
  dest=raw/name/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dest)
# Compiled source snapshots and generated source, without .a/.o/executables.
with tarfile.open(raw/'source-inputs.tar.gz','w:gz')as tar:
 for name in campaigns:
  for sub in ['baseline','candidate-inputs']:
   folder=a/name/sub
   if folder.exists():
    for p in folder.rglob('*'):
     if p.is_file():tar.add(p,arcname=f'{name}/{p.relative_to(a/name)}')
  for folder in(a/name).glob('*/api-sources'):
   for p in folder.rglob('*'):
    if p.is_file()and p.suffix in ['.c','.h','.json','.txt']:tar.add(p,arcname=f'{name}/{p.relative_to(a/name)}')
 for f in ['scripts/prepare-samples-link.py','scripts/native-samples-cache-env.sh','scripts/build-native-cache.sh','scripts/prepare-native-backend.py','patches/wgpu-native-command-cache.patch']:
  if (R/f).is_file():tar.add(R/f,arcname='tools/'+f)
index={str(p.relative_to(D)):{'bytes':p.stat().st_size,'sha256':sha(p)}for p in sorted(raw.rglob('*'))if p.is_file()};(D/'raw-index.json').write_text(json.dumps(index,indent=2)+'\n');print(len(index),'portable raw files',sum(v['bytes']for v in index.values()))
