from pathlib import Path
import json,hashlib
D=Path(__file__).resolve().parent;original=(D/'menu-controls-clip-driver.py').read_text();helpers=original[original.index('\n def find(kind,field):'):original.index("\n open_menu('Sim');tap('MenuItem','Pause')")];s=(D/'ui-auto-driver.py').read_text().replace("D/('ui01-build-'+phase+'.json')","D/'render-controls-build.json'").replace("('ui01-'+phase+'-runs')","'render-controls-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<120').replace("'180s overall automatic budget'","'120s Render budget'").replace("len(r['actions'])<60","len(r['actions'])<100").replace("len(r['screenshots'])<9","len(r['screenshots'])<4").replace('until(state,90)','until(state,25)').replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');","env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');")
s=s.replace("p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'","p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'\n p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'")
a=s.index("\n key('p');until(");b=s.index('\nexcept BaseException',a)
scenario=r'''
 key('p');until(lambda:state()['pause']==1,3)
 def renderer():
  p=dest/'api.jsonl'
  if not p.exists():return None
  with p.open('rb')as f:f.seek(max(0,p.stat().st_size-16384));lines=f.read().splitlines()
  for line in reversed(lines):
   try:q=json.loads(line)
   except json.JSONDecodeError:continue
   if q['api']=='actual_RenderFrame':return q
  return None
 def rendered(field,value):
  q=until(lambda:(q if(q:=renderer())and abs(q[field]-value)<1e-5 else None),3);r['render_proofs'].append({'field':field,'value':value,'actual_renderer':q});check(field+' actual renderer='+str(value),abs(q[field]-value)<1e-5);return q
 def menu_path(sub=None):
  open_menu('Render')
  if sub:open_menu(sub)
 def close_render():
  n=find('Menu','Render')
  if n['value']=='true':tap('Menu','Render')
 def edit_float(field,value,api):
  n=find('SliderFloat',field);hover(n['rect'][0]+12,(n['rect'][1]+n['rect'][3])/2)
  def send():
   xdo('keydown','Control_L');held.add('Control_L');xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1');xdo('keyup','Control_L');held.remove('Control_L');frames();xdo('key','ctrl+a');xdo('type','--clearmodifiers',str(value));xdo('key','Return')
  action('edit Render '+field+'='+str(value),send,n);n=find('SliderFloat',field);check(field+' displayed value',abs(float(n['value'])-value)<1e-5);rendered(api,value)
 r['render_proofs']=[];initial=until(renderer,3);r['initial_renderer']=initial;screenshot('default')
 for field,api,sub in [('Shadows','disableShadows',None),('Enable','disableAmbientOcclusion','GTAO'),('IBL','ibl',None)]:
  menu_path(sub);tap('MenuItem',field);rendered(api,1-initial[api]);menu_path(sub);tap('MenuItem',field);rendered(api,initial[api])
 menu_path();tap('MenuItem','Hull Edges');rendered('hullEdges',1-initial['hullEdges']);menu_path();tap('MenuItem','Edge Convexity');rendered('edgeConvexity',1-initial['edgeConvexity']);menu_path();tap('MenuItem','Edge Convexity');rendered('edgeConvexity',initial['edgeConvexity']);menu_path();tap('MenuItem','Hull Edges');rendered('hullEdges',initial['hullEdges'])
 for field,api,value in [('Start','shadowStart',3.0),('Range','shadowRange',80.0),('Distribution','shadowDistribution',.25)]:
  menu_path('Cascades');edit_float(field,value,api);edit_float(field,initial[api],api);close_render()
 menu_path('GTAO');old=int(initial['gtaoQuality']);tap('Combo','Quality');action('GTAO Quality Home Down Return',lambda:xdo('key','--delay',40,'Home','Down','Return'));rendered('gtaoQuality',1);menu_path('GTAO');tap('Combo','Quality');keys=['Home']+['Down']*old+['Return'];action('GTAO Quality restore',lambda:xdo('key','--delay',40,*keys));rendered('gtaoQuality',old);close_render()
 menu_path();edit_float('Sun',.25,'sun');edit_float('Sun',initial['sun'],'sun');edit_float('Exposure',-3.0,'exposure');close_render();screenshot('dark');menu_path();edit_float('Exposure',initial['exposure'],'exposure');close_render()
 for mode,name in enumerate(['0 - lit','1 - view-space distance','2 - CSM cascade','3 - view-space normal','4 - raw GTAO']):
  menu_path('Debug View');tap('RadioButton',name);rendered('debugMode',mode);n=find('RadioButton',name);check(name+' selected',n['value']=='true');close_render()
  if mode==3:screenshot('normal')
 menu_path('Debug View');tap('RadioButton',['0 - lit','1 - view-space distance','2 - CSM cascade','3 - view-space normal','4 - raw GTAO'][int(initial['debugMode'])]);rendered('debugMode',initial['debugMode']);close_render();final=renderer();r['final_renderer']=final;check('all renderer defaults restored',all(abs(initial[k]-final[k])<1e-5 for k in initial if k!='api'));screenshot('restored')
 from PIL import Image,ImageStat,ImageChops
 def roi(name):
  im=Image.open(dest/(name+'.png')).convert('RGB');w,h=im.size;return im.crop((round(w*.15),round(h*.4),round(w*.7),round(h*.85)))
 baseline=roi('default');dark=roi('dark');normal=roi('normal');bright=sum(ImageStat.Stat(baseline).mean)/3;dim=sum(ImageStat.Stat(dark).mean)/3;delta=sum(ImageStat.Stat(ImageChops.difference(baseline,normal)).mean)/3;r['image_effects']={'baseline_mean':bright,'dark_mean':dim,'normal_mean_abs_difference':delta};check('Exposure changes actual image luminance',dim<bright*.85);check('normal debug changes actual image',delta>15)
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='PASS_SHARED_RENDER_CONTROLS_ACTUAL_BOUNDARY';save();emit('Render actual parameters and image effects verified; defaults restored')
'''
s=s[:a]+helpers+scenario+s[b:];compile(s,str(D/'render-controls-driver.py'),'exec');(D/'render-controls-driver.py').write_text(s);(D/'render-controls-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','sha256':hashlib.sha256(s.encode()).hexdigest(),'apps_CPUfirst':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'seconds_each':120,'ready_seconds':25,'screens_each':4,'actions_each':100,'retries':0,'scope':'Render menu settings atactualRenderFrame +Exposure/normalimageeffects, initialdefaultsrestored. NotUI03overlayrepair.'},indent=2)+'\n')
s=(D/'run-menu-controls-clip.py').read_text().replace("'menu-controls-clip-matrix.json'","'render-controls-matrix.json'").replace("D/'menu-controls-clip-driver.py'","D/'render-controls-driver.py'").replace("raw/menu-controls-clip-build","raw/render-controls-build").replace("'after',label,label+'-clip'","'after',label").replace("'PASS_FIVE_ROUTES_SIM_VIEW_SUBSET'","'PASS_FIVE_ROUTES_RENDER_CONTROLS'");compile(s,str(D/'run-render-controls.py'),'exec');(D/'run-render-controls.py').write_text(s)
print('Renderdriver syntax/protocol frozen')
