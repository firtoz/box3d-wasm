from pathlib import Path
import json,hashlib,shutil,subprocess,io,datetime
from PIL import Image,ImageDraw
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-scene-captures';B=R/'benchmarks/production-readiness/pr02-world-lifetime-scene-captures-2026-10-02';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
p=json.loads((A/'protocol-before-captures.json').read_text());state=json.loads((A/'receipt.json').read_text());assert state['status']=='captured';assert len(state['results'])==42 and all(x['status']=='pass' for x in state['results']);native=json.loads((A/'native/receipt.json').read_text());assert native['status']=='captured' and len(native['results'])==12
B.mkdir(exist_ok=False);raw=B/'raw';raw.mkdir();write=lambda f,d:f.write_text(json.dumps(d,indent=2)+'\n')
# Append-only native per-scene-family manifests, with actual viewer/build proof.
np=json.loads((A/'native/protocol.json').read_text())
for kind,v in np['viewers'].items():
 rows=[x for x in native['results'] if x['kind']==kind];first=A/'native'/kind/np['scenes'][0]['id']/'launch.json';dest=R/'recordings/snapshots'/v['label']/'compound-properties-recording-manifest.json';assert not dest.exists()
 write(dest,{'recording_started_utc':datetime.datetime.fromtimestamp(first.stat().st_mtime,datetime.timezone.utc).isoformat(),'timestamp_source':'first launch.json persisted modification time, chronology only','purpose':np['purpose'],'protocol_sha256':sha(A/'native/protocol.json'),'viewer':v,'settings':np['settings'],'clips':rows,'coverage':'visual/finite health only, not final physics or performance'})
# Preserve complete original CPU column separately; unchanged unrelated clips keep identities.
for f in A.rglob('*'):
 if not f.is_file() or any(x in f.parts for x in ['standard-pipelines','native-pipelines','__pycache__']):continue
 if f.suffix in ['.a','.o','.so'] or f.name in ['gpu-physics-frozen','samples_cpu','samples_both']:continue
 dest=raw/'capture'/f.relative_to(A);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(f,dest)
cb=R/'artifacts/production-readiness/pr02-world-lifetime-capture-build'
for f in cb.iterdir():
 if f.is_file() and f.name!='gpu-physics-frozen':shutil.copyfile(f,raw/f.name)
# Original viewer composite parents and receipts preserve full build provenance.
for kind,v in np['viewers'].items():
 d=json.loads((R/v['receipt']).read_text());target=raw/('viewer-origin-'+kind);target.mkdir()
 for key in ['origin_viewer_receipt','origin_parent_receipt']:shutil.copyfile(d[key],target/(key+'.json'))
checks=[]
for kind in ['cpu','gpu']:
 label=p[kind+'_label'];dest=raw/'clips'/kind;dest.mkdir(parents=True)
 for scene in p['standard_scenes']+[x['id'] for x in p['compound_scenes']]:
  clip=R/'recordings/snapshots'/label/(scene+'.mp4');shutil.copyfile(clip,dest/clip.name)
  probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(clip)],text=True));v=next(x for x in probe['streams'] if x['codec_type']=='video');assert (v['width'],v['height'],v['r_frame_rate'])==(1280,720,'30/1');assert int(v['nb_frames'])>=300
  checks.append({'engine':kind,'scene':scene,'sha256':sha(clip),'bytes':clip.stat().st_size,'stream':v})
 for f in (R/'recordings/snapshots'/label).glob('*.json'):
  if f.name.startswith('metrics-') or f.name.endswith('-recording-manifest.json'):shutil.copyfile(f,dest/f.name)
write(raw/'clip-checks.json',{'clips':checks,'status':'52 clips validate; manual visual review pending','budget':p['budget']})
review=B/'review';review.mkdir()
def frame(clip,sec,width):
 data=subprocess.check_output(['ffmpeg','-v','error','-ss',str(sec),'-i',str(clip),'-frames:v','1','-vf',f'scale={width}:-1','-f','image2pipe','-vcodec','png','-']);return Image.open(io.BytesIO(data)).convert('RGB')
standard_inputs=json.loads((A/'standard-review/inputs.json').read_text())
for clip in checks:
 if clip['scene'] in p['standard_scenes']:assert standard_inputs[clip['engine']+'/'+clip['scene']]==clip['sha256']
for page in range(4):shutil.copyfile(A/'standard-review'/f'standard-{page+1}.png',review/f'standard-{page+1}.png')
for scene in p['compound_scenes']:
 sheet=Image.new('RGB',(1280,430),'#17212b');d=ImageDraw.Draw(sheet);d.text((8,6),scene['name']+' | real CPU above; combined CPU(left)/GPU(right) below; completed-scene window',fill='white')
 for row,kind in enumerate(['cpu','gpu']):
  entry=next(x for x in native['results'] if x['kind']==kind and x['scene']==scene['id']);health=json.loads((A/'native'/kind/scene['id']/'health.json').read_text());span=sum(f['cadence_ms'] for f in health['frames'])/1000;start=max(0,entry['duration_seconds']-span)
  for col,fraction in enumerate([.12,.3,.5,.7,.93]):
   sheet.paste(frame(raw/'clips'/kind/(scene['id']+'.mp4'),start+span*fraction,256),(col*256,30+row*200));d.text((col*256+6,177+row*200),f'{kind.upper()} at {fraction:.0%}',fill=(71,164,240) if kind=='cpu' else (242,153,62))
 sheet.save(review/(scene['id']+'.png'))
index={str(f.relative_to(B)):sha(f) for f in sorted(raw.rglob('*')) if f.is_file()};write(B/'raw-index.json',index);shutil.copyfile(A/'protocol-before-captures.json',B/'protocol.json')
print('Published',len(index),'raw files and52clips; manual review pending',flush=True)
