#!/usr/bin/env python3
"""Create portable first/last comparison sheets from every frozen standard clip."""
from pathlib import Path
import hashlib,json,subprocess,tempfile
from PIL import Image,ImageDraw,ImageFont
HERE=Path(__file__).resolve().parent
rows=json.loads((HERE/'recordings.json').read_text());scenes=[r['scene'] for r in rows if r['engine']=='cpu'];font=ImageFont.truetype('/usr/share/fonts/TTF/DejaVuSans.ttf',18)
out=HERE/'recording-review';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='pr01-review-') as tmp:
 for group in range(4):
  canvas=Image.new('RGB',(1920,5*320),'white');draw=ImageDraw.Draw(canvas)
  for row,scene in enumerate(scenes[group*5:(group+1)*5]):
   draw.text((12,row*320+8),scene,fill='black',font=font)
   for col,(engine,frame) in enumerate([('cpu',0),('cpu',299),('gpu',0),('gpu',299)]):
    clip=HERE/'recordings'/engine/f'{scene}.mp4';png=Path(tmp)/f'{engine}-{scene}-{frame}.png'
    cmd=['ffmpeg','-hide_banner','-loglevel','error','-i',str(clip),'-vf',f'select=eq(n\\,{frame})','-frames:v','1',str(png)];subprocess.run(cmd,check=True)
    im=Image.open(png);im.thumbnail((480,270));canvas.paste(im,(col*480,row*320+50))
    draw.text((col*480+12,row*320+28),f'{"Box3D CPU" if engine=="cpu" else "GPU"}, frame {frame}',fill='#d55e00' if engine=='cpu' else '#0072b2',font=font)
  canvas.save(out/f'group-{group+1}.png')
 print('Prepared all20 first/last comparison sheets')
