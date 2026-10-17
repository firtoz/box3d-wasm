from pathlib import Path
import json,hashlib
D=Path(__file__).resolve().parent;s=(D/'ui-auto-driver.py').read_text().replace("D/('ui01-build-'+phase+'.json')","D/'solver-controls-build.json'").replace("('ui01-'+phase+'-runs')","'solver-controls-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<60').replace("'180s overall automatic budget'","'60s solvercontrol budget'").replace("len(r['actions'])<60","len(r['actions'])<45").replace("len(r['screenshots'])<9","len(r['screenshots'])<2").replace('until(state,90)','until(state,25)').replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');","env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');")
s=s.replace("p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'","p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'\n p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'")
a=s.index("\n key('p');until(");b=s.index('\nexcept BaseException',a)
scenario=r'''
 key('p');until(lambda:(s if(s:=state())['pause']==1 else None),3)
 def api():
  p=dest/'api.jsonl';return [json.loads(x)for x in p.read_text().splitlines()]if p.exists()else []
 def edit(kind,label,value):
  n=item(kind,label);x=n['rect'][0]+12;y=(n['rect'][1]+n['rect'][3])/2;hover(x,y)
  def send():
   xdo('keydown','Control_L');held.add('Control_L');xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1');xdo('keyup','Control_L');held.remove('Control_L');frames();xdo('key','ctrl+a');xdo('type','--clearmodifiers',str(value));xdo('key','Return')
  action('edit '+label+'='+str(value),send,n);q=frames();v=next(x for x in q['items']if x['kind']==kind and x['label']==label);check('display '+label+'='+str(value),abs(float(v['value'])-value)<1e-5);return q
 initial={label:float(item(kind,label)['value'])for kind,label in [('SliderInt','Sub-steps##Solver'),('SliderFloat','Hertz##Solver'),('SliderInt','Workers##Solver'),('SliderFloat','Recycle##Solver')]};r['initial_controls']=initial
 edit('SliderInt','Sub-steps##Solver',7);edit('SliderFloat','Hertz##Solver',120);check('context hertz/substeps updated',state()['hertz']==120 and state()['substeps']==7)
 old=state()['sample_step'];index=len(api());key('period');until(lambda:(s if(s:=state())['sample_step']==old+1 else None),3);rows=api()[index:];r['edited_step_calls']=rows
 names=['b3World_Step']+(['gpu_b3_world_step']if label!='cpu'else [])+(['cpu_b3World_Step']if 'both'in label else [])
 for name in names:check(name+' edited args',any(x['api']==name and abs(x['value']-1/120)<1e-8 and x['substeps']==7 for x in rows))
 index=len(api());edit('SliderFloat','Recycle##Solver',3.5);rows=api()[index:];r['edited_recycle_calls']=rows
 names=['b3World_SetContactRecycleDistance']+(['gpu_b3_world_set_contact_recycle_distance']if label!='cpu'else [])+(['cpu_b3World_SetContactRecycleDistance']if 'both'in label else [])
 for name in names:check(name+' centimeters→meters',any(x['api']==name and abs(x['value']-.035)<1e-7 for x in rows))
 old=state()['world'];workers=2 if initial['Workers##Solver']!=2 else 3;edit('SliderInt','Workers##Solver',workers);until(lambda:(s if(s:=state())['world']!=old else None),3);check('workers creates new world',state()['world']!=old);check('edited settings retained across worker restart',state()['hertz']==120 and state()['substeps']==7);screenshot('edited')
 edit('SliderInt','Workers##Solver',int(initial['Workers##Solver']));edit('SliderFloat','Recycle##Solver',initial['Recycle##Solver']);edit('SliderFloat','Hertz##Solver',initial['Hertz##Solver']);edit('SliderInt','Sub-steps##Solver',int(initial['Sub-steps##Solver']));q=frames();r['restored_controls']={x['label']:x['value']for x in q['items']if x['label']in initial};check('all original slider defaults restored',all(abs(float(r['restored_controls'][k])-v)<1e-5 for k,v in initial.items()));screenshot('restored')
 old=state()['sample_step'];index=len(api());key('period');until(lambda:(s if(s:=state())['sample_step']==old+1 else None),3);rows=api()[index:];r['restored_step_calls']=rows
 for name in ['b3World_Step']+(['gpu_b3_world_step']if label!='cpu'else [])+(['cpu_b3World_Step']if 'both'in label else []):check(name+' default args restored',any(x['api']==name and abs(x['value']-1/initial['Hertz##Solver'])<1e-8 and x['substeps']==initial['Sub-steps##Solver']for x in rows))
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='PASS_SHARED_SOLVER_SLIDERS_ACTUAL_API';save();emit('Solver sliders actual API and defaults pass')
'''
s=s[:a]+scenario+s[b:];(D/'solver-controls-driver.py').write_text(s);(D/'solver-controls-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','sha256':hashlib.sha256(s.encode()).hexdigest(),'apps_CPUfirst':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'seconds_each':60,'ready_seconds':25,'screens_each':2,'actions_each':45,'retries':0},indent=2)+'\n')
