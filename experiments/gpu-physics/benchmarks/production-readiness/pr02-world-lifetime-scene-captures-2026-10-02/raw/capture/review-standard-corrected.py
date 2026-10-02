from pathlib import Path
import json,hashlib,subprocess,io
from PIL import Image,ImageDraw
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-scene-captures';p=json.loads((A/'protocol-before-captures.json').read_text());s=json.loads((A/'receipt.json').read_text());out=A/'standard-review-corrected';out.mkdir(exist_ok=False);sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs={}
def still(clip,sec):
 data=subprocess.check_output(['ffmpeg','-v','error','-ss',str(sec),'-i',str(clip),'-frames:v','1','-vf','scale=280:-1','-f','image2pipe','-vcodec','png','-']);return Image.open(io.BytesIO(data)).convert('RGB')
for page in range(4):
 sheet=Image.new('RGB',(960,1880),'#17212b');d=ImageDraw.Draw(sheet);d.text((8,7),'Real CPU above (blue labels), ordinary Vulkan GPU below (orange); frame0 / frame150 / frame299',fill='white')
 for row,scene in enumerate(p['standard_scenes'][page*5:(page+1)*5]):
  y=30+row*370;d.text((8,y),scene,fill='white')
  for kind,k in [('cpu',0),('gpu',1)]:
   clip=R/'recordings/snapshots'/p[kind+'_label']/(scene+'.mp4');entry=next(x for x in s['results'] if x['kind']==kind and x['scene']==scene);assert entry['status']=='pass' and sha(clip)==entry['clip_sha256'];inputs[kind+'/'+scene]=sha(clip)
   for col,sec in enumerate([0,5,299/30]):sheet.paste(still(clip,sec),(col*320,y+22+k*170))
   d.text((842,y+22+k*170),kind.upper(),fill=(71,164,240) if kind=='cpu' else (242,153,62))
 sheet.save(out/f'standard-{page+1}.png')
(out/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n');print('Four derived standard sheets;40 original clip hashes verified')
