from pathlib import Path
import json,hashlib,subprocess,io
from PIL import Image,ImageDraw
root=Path.cwd();out=root/'artifacts/production-readiness/pr02-diagnostics';protocol=json.loads((out/'recording-protocol.json').read_text());rows=[]
for kind,label in [('cpu','000-box3d-cpu'),('gpu','2026-10-01-native-diagnostics')]:
 for half in range(2):
  scenes=protocol['scenes'][half*10:(half+1)*10];sheet=Image.new('RGB',(640,10*205),'#17212b');draw=ImageDraw.Draw(sheet)
  for i,scene in enumerate(scenes):
   p=root/f'recordings/snapshots/{label}/{scene}.mp4';probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-select_streams','v:0','-show_streams','-of','json',str(p)]));v=probe['streams'][0]
   assert int(v['nb_frames'])==300 and (v['width'],v['height'],v['r_frame_rate'])==(1280,720,'30/1')
   rows.append({'engine':kind,'scene':scene,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size,'stream':v})
   draw.text((5,i*205+5),f'{kind}: {scene} — first / last',fill='white')
   for col,frame in enumerate([0,299]):
    raw=subprocess.check_output(['ffmpeg','-v','error','-i',str(p),'-vf',f'select=eq(n\\,{frame})','-frames:v','1','-f','image2pipe','-vcodec','png','-'])
    im=Image.open(io.BytesIO(raw)).resize((320,180));sheet.paste(im,(col*320,i*205+25))
  sheet.save(out/f'review-{kind}-{half+1}.png')
(out/'recording-checks.json').write_text(json.dumps({'clips':rows,'budget':protocol['budget'],'status':'40 complete clips; visual review pending'},indent=2)+'\n')
print('40 clips validate: 300 frames,1280x720,30fps; four review sheets written')
