from pathlib import Path
from PIL import Image,ImageDraw
import sys
p=Path(__file__).resolve().parent/sys.argv[1]
a=Image.new('RGB',(1056,720),'white');draw=ImageDraw.Draw(a)
for i,n in enumerate(['paused','flags-off','restart-off','flags-on']):
 im=Image.open(p/(n+'.png'));a.paste(im.crop((1014,25,1278,719)),(i*264,26));draw.text((i*264+4,5),n,fill='black')
a.save(p/'review-controls.png')
a=Image.new('RGB',(1000,690),'white');draw=ImageDraw.Draw(a)
for i,n in enumerate(['profile','counters','frame-time']):
 im=Image.open(p/(n+'.png'));a.paste(im.crop((5,505,1005,720)),(0,i*230+15));draw.text((4,i*230),n,fill='black')
a.save(p/'review-metrics.png')
