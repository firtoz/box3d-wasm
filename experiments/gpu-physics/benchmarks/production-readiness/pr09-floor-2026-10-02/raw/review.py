from pathlib import Path
import subprocess,json
from PIL import Image,ImageDraw
R=Path.cwd();O=R/'benchmarks/production-readiness/pr09-floor-2026-10-02';A=R/'artifacts/production-readiness/pr09-floor';p=json.loads((A/'before/protocol.json').read_text());viewers={'cpu':p['viewers']['cpu'],**p['gpu_viewers']};review=[]
for scene in p['scenes']:
 images=[]
 for key,v in viewers.items():
  clip=R/'recordings/snapshots'/v['label']/(scene['id']+'.mp4');health=A/'before'/key/scene['id']/'health.json'
  if not health.exists():continue
  probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(clip)]));duration=float(probe['streams'][0]['duration']);time=max(.1,duration-2)
  image=O/'review'/(key+'-'+scene['id']+'.png');image.parent.mkdir(exist_ok=True)
  subprocess.run(['ffmpeg','-v','error','-y','-ss',str(time),'-i',str(clip),'-frames:v','1','-vf','scale=640:360',str(image)],check=True)
  images.append((key,Image.open(image).convert('RGB')));review.append({'scene':scene['id'],'viewer':key,'clip':str(clip.relative_to(R)),'duration':duration,'sample_seconds':time,'stage':'late sample; approximate physics checkpoint, startup included'})
 if images:
  sheet=Image.new('RGB',(640,len(images)*388),'white');draw=ImageDraw.Draw(sheet)
  for i,(key,img) in enumerate(images):draw.text((8,i*388+7),key,fill='black');sheet.paste(img,(0,i*388+28))
  sheet.save(O/'review'/('before-'+scene['id']+'.png'))
(O/'raw/review-sampling.json').write_text(json.dumps(review,indent=2)+'\n')
