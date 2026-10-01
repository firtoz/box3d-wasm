from pathlib import Path
import shutil,json,hashlib
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-diagnostics';report=root/'benchmarks/production-readiness/pr02-diagnostics-2026-10-01';raw=report/'raw'
def copy(source,dest):dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dest)
for name in ['recording-protocol.json','standard-recordings.json','recording-checks.json','recording-label-map.json','previous-cpu-column.json','record-standard.py','review-recordings.py','viewer-interact.py','viewer-interaction-protocol.json','viewer-interaction-result.json','package-recordings.py','build-recorder.py','build-oracle.py']:
 copy(out/name,raw/name)
for p in out.glob('record-standard-*.log'):copy(p,raw/p.name)
for p in out.glob('review-*.png'):copy(p,raw/p.name)
for name in ['receipt.json','oracle-receipt.json','build.log','compiled-inputs.tar.gz','oracle-inputs.tar.gz','oracle-build-1.log','oracle-build-2.log']:
 p=out/'recording-build'/name
 if p.exists():copy(p,raw/'recording-build'/name)
for p in (out/'viewer-interactions').rglob('*'):
 if p.is_file():copy(p,raw/'viewer-interactions'/p.relative_to(out/'viewer-interactions'))
for kind,label in [('cpu','000-box3d-cpu'),('gpu','2026-10-01-truthful-native-diagnostics')]:
 for scene in json.loads((out/'recording-protocol.json').read_text())['scenes']:
  for p in (root/f'recordings/snapshots/{label}').glob(scene+'*'):
   if p.is_file():copy(p,raw/'recordings'/kind/p.name)
copy(root/'artifacts/production-readiness/pr01-speculative/handoff/native-scene-cpu/build.json',raw/'viewer-interactions/cpu/compiled-build.json')
index=json.loads((report/'raw-index.json').read_text());old={x['path']:x for x in index['entries']};entries=[]
for p in sorted(raw.rglob('*')):
 if p.is_file():
  path=str(p.relative_to(report));entry={'path':path,'portable_sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
  if path in old:
   assert old[path]['portable_sha256']==entry['portable_sha256'],path
   entry=old[path]
  entries.append(entry)
index.update(scope='Focused diagnostics API checks and CPU-first recordings; actual tab-selection failure retained; broader PR02 open',entries=entries);(report/'raw-index.json').write_text(json.dumps(index,indent=2)+'\n')
print(len(entries),'portable files',sum(p.stat().st_size for p in raw.rglob('*') if p.is_file())//1024//1024,'MiB')
