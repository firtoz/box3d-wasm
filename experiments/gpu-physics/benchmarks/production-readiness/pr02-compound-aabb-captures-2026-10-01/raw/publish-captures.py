from pathlib import Path
import hashlib,json,shutil,subprocess
from PIL import Image,ImageDraw
r=Path.cwd();a=r/'artifacts/production-readiness/pr02-compound-aabb-captures';out=r/'benchmarks/production-readiness/pr02-compound-aabb-captures-2026-10-01';out.mkdir(exist_ok=False);raw=out/'raw';raw.mkdir()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
p=json.loads((a/'protocol.json').read_text());state=json.loads((a/'receipt.json').read_text());assert state['status'] in ['captured','stopped']
files=[a/'protocol.json',a/'receipt.json',a/'adapter.txt',a/'capture-inputs.tar.gz',a/'freeze-captures.py',a/'build-cpu-viewer.py',a/'publish-captures.py',a/'supplement-viewer-dependencies.py',a/'viewer-dependencies.json',a/'viewer-dependencies.tar.gz']
files += list((a/'cpu-viewer').glob('*.*'))
files += list(a.glob('*-xvfb.log'))
for kind in ['cpu','gpu']:
 for d in (a/kind).glob('*'):
  files += [x for x in d.iterdir() if x.is_file()]
  if (d/'cwd/settings.ini').is_file():files.append(d/'cwd/settings.ini')
for f in sorted(set(files)):
 dest=raw/f.relative_to(a);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,dest)
shutil.copyfile(a/'protocol.json',out/'protocol.json')
shutil.copyfile(a/'implementation-review.md',out/'implementation-review.md')
shutil.copyfile(a/'validate-draft.py',out/'validate.py')
# Append-only per-campaign manifests; the established CPU column is preserved.
for kind,v in p['viewers'].items():
 rows=[x for x in state['results'] if x['kind']==kind]
 manifest={'purpose':p['purpose'],'protocol_sha256':sha(a/'protocol.json'),'viewer':v,'settings':p['settings'],'clips':rows,'coverage':'scene recording/finite health only; no physical or timing acceptance'}
 dest=r/'recordings/snapshots'/v['label']/'native-compound-aabb-manifest.json';assert not dest.exists();dest.write_text(json.dumps(manifest,indent=2)+'\n')
 index={str(f.relative_to(out)):sha(f) for f in sorted(raw.rglob('*')) if f.is_file()}
(out/'raw-index.json').write_text(json.dumps(index,indent=2)+'\n')
# CPU-first contact sheets, sampling each complete clip at five completed scene window times.
review=out/'review';review.mkdir()
for scene in p['scenes']:
 sheet=Image.new('RGB',(1280,430),(16,18,23));draw=ImageDraw.Draw(sheet)
 draw.text((12,5),scene['name']+' | CPU above, native GPU below | visual/health, no timing claim',fill='white')
 for row,kind in enumerate(['cpu','gpu']):
  entry=next((x for x in state['results'] if x['kind']==kind and x['scene']==scene['id']),None)
  if not entry or 'duration_seconds' not in entry:continue
  video=r/'recordings/snapshots'/p['viewers'][kind]['label']/(scene['id']+'.mp4')
  health=json.loads((a/kind/scene['id']/'health.json').read_text())
  completed_span=sum(f['cadence_ms'] for f in health['frames'])/1000
  visible_start=max(0,entry['duration_seconds']-completed_span)
  for column,fraction in enumerate([.12,.3,.5,.7,.93]):
   still=review/f"{scene['id']}-{kind}-{column}.png"
   subprocess.run(['ffmpeg','-v','error','-ss',str(visible_start+completed_span*fraction),'-i',str(video),'-frames:v','1','-vf','scale=256:144',str(still)],check=True)
   sheet.paste(Image.open(still),(column*256,30+row*200))
   draw.text((column*256+6,177+row*200),f'{kind.upper()} at {fraction:.0%} completed scene window',fill=(71,164,240) if kind=='cpu' else (242,153,62))
 sheet.save(review/(scene['id']+'.png'))
print('Published',len(index),'raw files; status',state['status'],flush=True)
