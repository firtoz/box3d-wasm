#!/usr/bin/env python3
"""Archive existing captures and create visual-review sheets; launches no physics."""
import hashlib,json,shutil,subprocess,io
from pathlib import Path
from PIL import Image,ImageDraw
R=Path(__file__).resolve().parent
E=R.parents[2]
A=E/'artifacts/production-readiness/pr04-distance-captures'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,x):p.write_text(json.dumps(x,indent=2)+'\n')
def copy(a,b):b.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(a,b)
p=json.loads((A/'protocol.json').read_text());s=json.loads((A/'receipt.json').read_text())
assert s['status']=='captured' and len(s['results'])==15
for f in A.rglob('*'):
 if f.is_file() and not f.is_symlink() and 'cwd' not in f.parts and not any('pipelines' in x for x in f.parts):copy(f,R/'raw/capture'/f.relative_to(A))
for n in p['source_files']:
 copy((E/n).resolve(),R/'raw/sources'/Path(n).name)
viewers={'cpu':p['viewers']['cpu'],**p['gpu_viewers']}
checks=[];samples=[]
for key,v in viewers.items():
 copy(E/v['receipt'],R/'raw/viewer-receipts'/f'{key}.json')
 manifest=E/'recordings/snapshots'/v['label']/'distance-clamp-recording-manifest.json'
 copy(manifest,R/'raw/manifests'/f'{key}.json')
 for scene in p['scenes']:
  name=scene['id'];d=A/key/name
  video=E/'recordings/snapshots'/v['label']/(name+'.mp4')
  target=R/'raw/clips'/key/(name+'.mp4');copy(video,target)
  h=json.loads((d/'health.json').read_text());c=json.loads((d/'capture.json').read_text());assert sha(video)==c['video_sha256']
  checks.append({'viewer':key,'scene':name,'path':str(target.relative_to(R)),'sha256':sha(video),'stream':c['probe']['streams'][0]})
  # Approximate time from the end of the recording using callback cadence.
  # Encoding starts before process initialization; this is not exact frame synchronization.
  fs=h['frames'];tail=sum(fs[-1].get(x,0) for x in ['physics_ms','setters_ms','profile_ms','pick_ms','draw_ms','render_ms','ui_ms','commit_ms','limiter_ms','health_ms'])/1000
  for step in [25,100,200,275]:
   back=sum(f['cadence_ms'] for f in fs[step:])/1000+tail
   t=max(0,c['duration_seconds']-back-.5)
   raw=subprocess.check_output(['ffmpeg','-v','error','-ss',str(t),'-i',str(video),'-frames:v','1','-vf','scale=400:225','-f','image2pipe','-vcodec','png','-'])
   out=R/'raw/review-frames'/key/name/f'{step:03}.png';out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(raw)
   samples.append({'viewer':key,'scene':name,'approximate_step':step,'video_seconds':t,'path':str(out.relative_to(R)),'sha256':sha(out)})
write(R/'raw/clip-checks.json',checks);write(R/'raw/review-sampling.json',{'method':'End-aligned callback-cadence estimate, not exact video/physics synchronization','samples':samples})
for scene in p['scenes']:
 sheet=Image.new('RGB',(2000,1000),'white');draw=ImageDraw.Draw(sheet)
 for col,key in enumerate(viewers):
  for row,step in enumerate([25,100,200,275]):
   y=row*250;draw.text((col*400+5,y+5),f'{key} / approximately step {step}',fill='black')
   sheet.paste(Image.open(R/'raw/review-frames'/key/scene['id']/f'{step:03}.png'),(col*400,y+25))
 sheet.save(R/(scene['id']+'-review.png'))
write(R/'raw-index.json',{str(f.relative_to(R)):sha(f) for f in sorted((R/'raw').rglob('*')) if f.is_file()})
print('Archived',len(checks),'clips and',len(samples),'review frames; no engine launches')
