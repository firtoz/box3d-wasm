from pathlib import Path
import json,shutil,hashlib
R=Path.cwd();B=R/'benchmarks/production-readiness/pr03-baseline-fixtures-2026-10-02';assert not B.exists();B.mkdir();raw=B/'raw';raw.mkdir()
for name,target in [('pr03-baseline-builds','first-build'),('pr03-baseline-cpu-link-repair','dependency-repair'),('pr03-distance-baseline','distance'),('pr03-ragdoll-mesh-baseline','ragdoll-mesh')]:
 source=R/'artifacts/production-readiness'/name
 for f in source.rglob('*'):
  if not f.is_file() or f.suffix in ['.a','.o','.bin'] or f.name=='fixture' or 'legacy-evaluation' in f.parts:continue
  to=raw/target/f.relative_to(source);to.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,to)
# Keep strict evaluator outputs, but only one copy of each original CPU/GPU trace.
for cell in ['ordinary','native']:
 source=R/'artifacts/production-readiness/pr03-ragdoll-mesh-baseline/legacy-evaluation'/cell
 for name in ['stdout.log','stderr.log','result.json','frame-errors.json']:
  f=source/name
  if f.exists():to=raw/'ragdoll-mesh/legacy-evaluation'/cell/name;to.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,to)
for name in ['check-ragdoll-health.py','validate-ragdoll-reference.py','compare-frozen-contacts.py']:
 to=raw/'evaluators'/name;to.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(R/'scripts'/name,to)
for f in (R/'c_abi/fixtures/frozen-mesh').iterdir():
 if f.is_file():to=raw/'frozen-mesh-inputs'/f.name;to.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,to)
producers={'engine-build.json':R/'artifacts/production-readiness/pr02-world-lifetime-artifact-repair/build-receipt.json','viewer-parent.json':R/'artifacts/production-readiness/pr02-world-lifetime-viewer-builds/receipt.json','cpu-applicability.json':R/'artifacts/production-readiness/pr02-world-lifetime-scene-captures/native/cpu-applicability-receipt.json'}
for cell in ['ordinary-gpu','native-gpu','ordinary-both','native-both']:producers[cell+'.json']=R/('artifacts/production-readiness/pr02-world-lifetime-viewer-builds/'+cell+'/receipt.json')
for key,f in producers.items():to=raw/'producers'/key;to.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,to)
for name in ['origin_viewer_receipt.json','origin_parent_receipt.json']:shutil.copyfile(R/'benchmarks/production-readiness/pr02-world-lifetime-scene-captures-2026-10-02/raw/viewer-origin-cpu'/name,raw/'producers'/('cpu-'+name))
index={str(f.relative_to(B)):{'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'bytes':f.stat().st_size} for f in sorted(raw.rglob('*')) if f.is_file()};(B/'raw-index.json').write_text(json.dumps(index,indent=2)+'\n');print('Published',len(index),'portable rawfiles,',sum(x['bytes'] for x in index.values()),'bytes;no compiledbinaries/objects/archives')
