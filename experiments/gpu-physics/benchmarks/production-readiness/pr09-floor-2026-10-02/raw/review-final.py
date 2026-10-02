from pathlib import Path
import json,subprocess,html
from PIL import Image,ImageDraw
R=Path.cwd();O=R/'benchmarks/production-readiness/pr09-floor-2026-10-02';A=R/'artifacts/production-readiness/pr09-floor';before=json.loads((A/'before/protocol.json').read_text());after=json.loads((A/'after/protocol.json').read_text());sampling=[];columns=[('cpu','cpu',before['viewers']['cpu'])]+[(phase,key,v) for phase,p in [('after',after),('before',before)] for key,v in p['gpu_viewers'].items()]
assert json.loads((A/'after-remaining/receipt.json').read_text())['status']=='captured'
for scene in before['scenes']:
 for phase,p in [('before',before),('after',after)]:
  rows=[('cpu',before['viewers']['cpu'])]+list(p['gpu_viewers'].items())
  for stage,seconds in [('early',5),('late',2)]:
   sheet=Image.new('RGB',(640,388*5),'white');draw=ImageDraw.Draw(sheet)
   for i,(key,v) in enumerate(rows):
    clip=R/'recordings/snapshots'/v['label']/(scene['id']+'.mp4');probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(clip)]));duration=float(probe['streams'][0]['duration']);end=duration
    if phase=='after' and key=='ordinary-both' and scene['id']=='pr09-floor-mesh-grid':end=json.loads((A/'after/ordinary-both/pr09-floor-mesh-grid/capture.json').read_text())['review_window_end_seconds']
    point=max(.1,end-seconds);out=O/'review'/(phase+'-'+stage+'-'+key+'-'+scene['id']+'.png')
    subprocess.run(['ffmpeg','-v','error','-y','-ss',str(point),'-i',str(clip),'-frames:v','1','-vf','scale=640:360',str(out)],check=True)
    sampling.append({'phase':phase,'stage':stage,'viewer':key,'scene':scene['id'],'clip':str(clip.relative_to(R)),'duration':duration,'point_seconds':point,'image':str(out.relative_to(O)),'scope':'approximate callback checkpoint; full clip retains startup/shutdown. These samples cannot prove all-frame pixel/trajectory equivalence.'})
    draw.text((8,i*388+7),('real Box3D CPU reference' if key=='cpu' else phase+' '+key),fill='black');sheet.paste(Image.open(out).convert('RGB'),(0,i*388+28))
   sheet.save(O/'review'/(phase+'-'+stage+'-'+scene['id']+'.png'))
(O/'raw/review-sampling-final.json').write_text(json.dumps(sampling,indent=2)+'\n')
# Standalone portable grid, real CPU pinned first, latest repaired GPU before old GPU.
s='<meta charset="utf-8"><title>PR09 floor repair: CPU first</title><style>body{font:14px system-ui}table{border-collapse:collapse}td,th{padding:8px;border:1px solid #ccc}video{width:480px}th:first-child,td:first-child{position:sticky;left:0;background:white;z-index:1}</style><h1>PR09 floor allocation milestone</h1><p>Real Box3D CPU first; repaired ordinary/native GPU and combined; retained before captures. 60 paced physics steps, diagnostic graphics. Encoding at 30 FPS does not measure rendering FPS. Exact pixels and scene trajectories are outside this milestone.</p><table><thead><tr><th>Scene</th>'
for phase,key,v in columns:s+='<th>'+html.escape(phase+' '+key)+'</th>'
s+='</tr></thead><tbody>'
for scene in before['scenes']:
 s+='<tr><th>'+html.escape(scene['name'])+'</th>'
 for phase,key,v in columns:
  path='../../../recordings/snapshots/'+v['label']+'/'+scene['id']+'.mp4'
  if phase=='after' and key=='ordinary-both' and scene['id']=='pr09-floor-mesh-grid':path+='#t=0,11.257637739181519'
  s+='<td><video controls muted loop preload="metadata" src="'+html.escape(path)+'"></video></td>'
 s+='</tr>'
s+='</tbody></table>';(O/'compare.html').write_text(s)
