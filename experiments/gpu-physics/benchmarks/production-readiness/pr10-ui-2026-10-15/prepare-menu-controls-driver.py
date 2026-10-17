from pathlib import Path
import json,hashlib
D=Path(__file__).resolve().parent;s=(D/'ui-auto-driver.py').read_text().replace("D/('ui01-build-'+phase+'.json')","D/'menu-controls-build.json'").replace("('ui01-'+phase+'-runs')","'menu-controls-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<120').replace("'180s overall automatic budget'","'120s menucontrol budget'").replace("len(r['actions'])<60","len(r['actions'])<100").replace("len(r['screenshots'])<9","len(r['screenshots'])<3").replace('until(state,90)','until(state,25)').replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');","env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');")
s=s.replace("p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'","p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'\n p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'")
a=s.index("\n key('p');until(");b=s.index('\nexcept BaseException',a)
scenario=r'''
 def find(kind,field):
  q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==field and x['visible']];assert rows,(kind,field,'not submitted',[(x['kind'],x['label'],x['window'])for x in q['items']if x['kind']in ['Menu','MenuItem']]);return rows[-1]
 def tap(kind,field):
  n=find(kind,field);assert not n['disabled'];x=(n['rect'][0]+n['rect'][2])/2;y=(n['rect'][1]+n['rect'][3])/2;hover(x,y)
  def send():xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
  return action('menu '+field,send,n)
 def open_menu(field):
  n=find('Menu',field)
  if n['value']!='true':tap('Menu',field)
  until(lambda:any(x['kind']=='Menu'and x['label']==field and x['value']=='true'for x in ui()['items']),3)
 def draw():
  p=dest/'api.jsonl'
  if not p.exists():return None
  with p.open('rb')as f:f.seek(max(0,p.stat().st_size-16384));rows=f.read().splitlines()
  for line in reversed(rows):
   try:q=json.loads(line)
   except json.JSONDecodeError:continue
   if q['api']=='b3World_Draw':return q
  return None
 def draw_value(field,value):return until(lambda:(q if(q:=draw())and q[field]==value else None),3)
 open_menu('Sim');tap('MenuItem','Pause');until(lambda:state()['pause']==1,3);old=state()['sample_step'];frames();check('menu pause stable',state()['sample_step']==old);open_menu('Sim');tap('MenuItem','Single Step');until(lambda:state()['sample_step']==old+1,3);check('menu single step exact',state()['sample_step']==old+1)
 old=state()['world'];open_menu('Sim');tap('MenuItem','Restart');until(lambda:state()['world']!=old,25);until(lambda:any(x['window']=='Info'for x in ui()['items']),25);check('menu restart new world',state()['world']!=old)
 screenshot('initial');initial=draw();assert initial;r['initial_draw_flags']=initial;r['toggle_proofs']=[]
 flags=[('Shapes','shapes'),('Joints','joints'),('Joint Extras','extras'),('Bounds','bounds'),('Mass','mass'),('Sleep','sleep'),('Body Names','names'),('Graph Colors','graph'),('Islands','islands'),('Contact Points','contacts'),('Contact Normals','normals'),('Contact Features','features'),('Contact Forces','forces')]
 for field,flag in flags:
  old=initial[flag];open_menu('View');n=find('MenuItem',field);check(field+' UI initial state',n['value']==('true'if old else 'false'));tap('MenuItem',field);proof=draw_value(flag,1-old);r['toggle_proofs'].append({'item':field,'phase':'changed','actual_draw':proof});check(field+' actual draw changed',proof[flag]==1-old)
  if field=='Shapes':screenshot('shapes-hidden')
  open_menu('View');n=find('MenuItem',field);check(field+' UI changed state',n['value']==('false'if old else 'true'));tap('MenuItem',field);proof=draw_value(flag,old);r['toggle_proofs'].append({'item':field,'phase':'restored','actual_draw':proof});check(field+' actual draw restored',proof[flag]==old)
 open_menu('View');open_menu('Anchor');old=initial['anchorA'];tap('MenuItem','Anchor B'if old else 'Anchor A');proof=draw_value('anchorA',1-old);check('anchor actual draw changed',proof['anchorA']==1-old);r['anchor_changed']=proof
 open_menu('View');open_menu('Anchor');tap('MenuItem','Anchor A'if old else 'Anchor B');proof=draw_value('anchorA',old);check('anchor actual draw restored',proof['anchorA']==old)
 old=state()['show_metrics'];open_menu('View');tap('MenuItem','Diagnostics');until(lambda:state()['show_metrics']!=old,3);check('diagnostics menu opens metrics',state()['show_metrics']!=old);open_menu('View');tap('MenuItem','Diagnostics');until(lambda:state()['show_metrics']==old,3);check('diagnostics restores',state()['show_metrics']==old)
 final=draw();r['final_draw_flags']=final;check('all draw defaults restored',all(initial[k]==final[k]for k in ['shapes','joints','extras','bounds','mass','sleep','names','contacts','anchorA','graph','features','normals','forces','islands','jointScale','forceScale']));screenshot('restored')
 open_menu('Sim');tap('MenuItem','Quit');app.wait(timeout=5);check('menu quit clean',app.returncode==0);r['status']='PASS_SHARED_SIM_VIEW_SUBSET_ACTUAL_DRAW';save();emit('Sim/View actual draw and restored defaults pass')
'''
s=s[:a]+scenario+s[b:];s=s.replace("open_menu('Sim');tap('MenuItem','Quit');app.wait(timeout=5)","open_menu('Sim');n=find('MenuItem','Quit');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);xdo('click',1);app.wait(timeout=5)")
compile(s,str(D/'menu-controls-driver.py'),'exec');(D/'menu-controls-driver.py').write_text(s);(D/'menu-controls-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','sha256':hashlib.sha256(s.encode()).hexdigest(),'apps_CPUfirst':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'seconds_each':120,'ready_seconds':25,'screens_each':3,'actions_each':100,'retries':0,'tests':['SimPause/Step/Restart/Quit','13 View flagtoggle+restore mappedto actualb3World_Draw arguments','Anchor A/B actualdrawflag','Diagnostics visiblemetricstate','geometry hidden/restoredscreens']},indent=2)+'\n')
print('Menu driver syntax and frozen protocol PASS')
