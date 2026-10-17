from pathlib import Path
import json,hashlib
D=Path(__file__).resolve().parent;o=(D/'menu-controls-clip-driver.py').read_text();helpers=o[o.index('\n def find(kind,field):'):o.index("\n open_menu('Sim');tap('MenuItem','Pause')")];s=(D/'ui-auto-driver.py').read_text().replace("D/('ui01-build-'+phase+'.json')","D/'body-metadata-build.json'").replace("('ui01-'+phase+'-runs')","'body-view-controls-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<120').replace("'180s overall automatic budget'","'120s bodyview budget'").replace("len(r['actions'])<60","len(r['actions'])<100").replace("len(r['screenshots'])<9","len(r['screenshots'])<8").replace('until(state,90)','until(state,25)').replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');","env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');").replace("'Stacking/Single Box'","'Bodies/Body Type'")
s=s.replace("p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'","p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'\n p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'API cap'")
a=s.index("\n key('p');until(");b=s.index('\nexcept BaseException',a)
scenario=r'''
 key('p');until(lambda:state()['pause']==1,3);check('BodyType initial8bodies/shapes',state()['body_count']==8 and state()['shape_count']==8);r['findings']=[];r['type_cases']=[];r['enable_cases']=[];r['view_proofs']=[];initial_world=state()['world']
 def recorded(name,predicate,context):
  r['checks'][name]=bool(predicate)
  if not predicate:r['findings'].append({'check':name,'context':context})
  save();emit(('PASS 'if predicate else 'FINDING ')+name)
 def all_api():
  p=dest/'api.jsonl';return [json.loads(line)for line in p.read_text().splitlines()]if p.exists()else []
 def latest(name):
  p=dest/'api.jsonl'
  if not p.exists():return None
  with p.open('rb')as f:f.seek(max(0,p.stat().st_size-65536));lines=f.read().splitlines()
  for line in reversed(lines):
   try:q=json.loads(line)
   except json.JSONDecodeError:continue
   if q['api']==name:return q
  return None
 def metadata():
  q=frames();return until(lambda:(m if(m:=latest('actual_body_metadata'))and m['request']>=q['request']else None),3)
 def body(m,name,owner='public'):return next(x for x in m['watched']if x.get('name')==name and x['owner']==owner and x['valid'])
 def setting(kind,field,value):
  n=find(kind,field);hover(n['rect'][0]+12,(n['rect'][1]+n['rect'][3])/2)
  def send():
   xdo('keydown','Control_L');held.add('Control_L');xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1');xdo('keyup','Control_L');held.remove('Control_L');frames();xdo('key','ctrl+a');xdo('type','--clearmodifiers','--',str(value));xdo('key','Return')
  action('edit '+field+'='+str(value),send,n);n=find(kind,field);check(field+' displayed '+str(value),abs(float(n['value'])-value)<1e-5)
 def change_type(field,target):
  index=len(all_api());click('RadioButton',field);check(field+' radio selected',item('RadioButton',field)['value']=='true');rows=[x for x in all_api()[index:]if x['api']in ['b3Body_SetType','cpu_b3Body_SetType']];after=[x for x in rows if x['phase']=='after'];before=[x for x in rows if x['phase']=='before'];expected=10 if 'both'in label else 5;recorded(field+' exact setter group',len(after)==expected,{'rows':rows});recorded(field+' actual body types',len(after)==expected and all(x['body'].get('type')==target for x in after),{'rows':rows});pose_ok=all(max(abs(a-b)for a,b in zip(x['body']['position'],y['body']['position']))<1e-5 for x,y in zip(before,after)if x['body']['valid']and y['body']['valid']);recorded(field+' preserves body origins',pose_ok,{'rows':rows});r['type_cases'].append({'control':field,'target':target,'calls':rows,'after_metadata':metadata()});save();return rows
 def step_once():
  focus();before=state();index=len(all_api());t=time.monotonic();xdo('keydown','period');held.add('period');time.sleep(.04);xdo('keyup','period');held.remove('period');until(lambda:state()['sample_step']==before['sample_step']+1,25);frames();r['actions'].append({'kind':'controlled single step','state_before':before,'state_after':state(),'seconds':time.monotonic()-t});r.setdefault('step_calls',[]).append([x for x in all_api()[index:]if x['api']in ['b3World_Step','gpu_b3_world_step','cpu_b3World_Step']]);save()
 initial=metadata();r['initial_metadata']=initial;screenshot('initial');open_menu('View');open_menu('Transparency');tap('MenuItem','Dynamic');m=metadata();r['view_proofs'].append({'control':'Dynamic transparency on','metadata':m});recorded('dynamic transparency actual7instances',m['render_counts']['transparent']==7,{'metadata':m});screenshot('dynamic-transparent');open_menu('View');open_menu('Transparency');tap('MenuItem','Dynamic');m=metadata();recorded('dynamic transparency restores0',m['render_counts']['transparent']==0,{'metadata':m})
 change_type('Static',0);screenshot('static');change_type('Kinematic',1);open_menu('View');open_menu('Transparency');tap('MenuItem','Kinematic');m=metadata();r['view_proofs'].append({'control':'Kinematic transparency on','metadata':m});recorded('kinematic transparency actual5instances',m['render_counts']['transparent']==5,{'metadata':m});screenshot('kinematic-transparent');open_menu('View');open_menu('Transparency');tap('MenuItem','Kinematic');m=metadata();recorded('kinematic transparency restores0',m['render_counts']['transparent']==0,{'metadata':m});before=metadata();step_once();step_once();after=metadata();r['kinematic_motion']={'before':before,'after':after,'expected_dx':-.1}
 for owner in ['public']+(['mapped_cpu']if 'both'in label else []):
  b=body(before,'platform',owner);a=body(after,'platform',owner);recorded(owner+' commanded kinematic translation',abs(a['position'][0]-b['position'][0]+.1)<1e-3,{'before':b,'after':a,'steps':2,'dt':1/60,'vx':-3})
 change_type('Dynamic',2);screenshot('dynamic')
 for enabled in [False,True]:
  index=len(all_api());click('Checkbox','Enable');check('Enable display '+str(enabled),item('Checkbox','Enable')['value']==('true'if enabled else 'false'));names=['b3Body_Enable','cpu_b3Body_Enable']if enabled else ['b3Body_Disable','cpu_b3Body_Disable'];rows=[x for x in all_api()[index:]if x['api']in names];after=[x for x in rows if x['phase']=='after'];expected=6 if 'both'in label else 3;recorded('Enable actual '+str(enabled),len(after)==expected and all(x['body'].get('enabled')==enabled for x in after),{'rows':rows});r['enable_cases'].append({'enabled':enabled,'calls':rows,'metadata':metadata()})
  if not enabled:screenshot('disabled')
 recorded('body widgets keep world/counts',state()['world']==initial_world and state()['body_count']==8 and state()['shape_count']==8,{'state':state()})
 def radius():return float(re.search(r'radius m ([0-9.e+-]+)',next(x['label']for x in frames()['items']if x['label'].startswith('radius m ')))[1])
 before=metadata();recorded('framing has no body selection',not before['selected']['valid']and not before['selected_cpu']['valid'],{'metadata':before});open_menu('View');tap('MenuItem','Frame Camera');menu_radius=radius();key('f');key_radius=radius();r['camera_framing']={'menu_radius':menu_radius,'keyboard_radius':key_radius,'width':frames()['width'],'height':frames()['height'],'selected_metadata':before};recorded('menu Frame Camera matches F',abs(menu_radius-key_radius)<.2,r['camera_framing'])
 open_menu('View');open_menu('Scale');joint=float(find('InputFloat','Joint')['value']);force=float(find('InputFloat','Force')['value']);setting('InputFloat','Joint',joint*2);q=until(lambda:(q if(q:=draw())and abs(q['jointScale']-joint*2)<1e-5 else None),3);r['view_proofs'].append({'control':'Joint Scale','actual_draw':q,'metadata':metadata()});setting('InputFloat','Joint',joint);setting('InputFloat','Force',force*2);q=until(lambda:(q if(q:=draw())and abs(q['forceScale']-force*2)<1e-6 else None),3);r['view_proofs'].append({'control':'Force Scale','actual_draw':q,'metadata':metadata()});setting('InputFloat','Force',force);open_menu('View');tap('Menu','View')
 def select_query(query,count):
  old=state()['world'];key('ctrl+o');action('picker '+query,lambda:xdo('type','--clearmodifiers','--',query));key('Return');until(lambda:state()['world']!=old and state()['body_count']==count,25);until(lambda:any(w['name']=='Info'for w in ui()['windows']),25);return old
 select_query('CompoundHulls',1);baseline=metadata();open_menu('View');distance=float(find('SliderFloat','Draw Distance')['value']);setting('SliderFloat','Draw Distance',10);tap('Menu','View');culled=metadata();r['draw_distance_case']={'initial_distance':distance,'baseline':baseline,'culled':culled,'scene_state':state()};recorded('Draw Distance culls compound child instances',culled['render_counts']['opaque']<baseline['render_counts']['opaque'],r['draw_distance_case']);screenshot('culled');open_menu('View');setting('SliderFloat','Draw Distance',distance);tap('Menu','View');restored=metadata();r['draw_distance_case']['restored']=restored;recorded('Draw Distance restores compound instances',restored['render_counts']['opaque']==baseline['render_counts']['opaque'],r['draw_distance_case'])
 select_query('BodyType',8);q=frames();check('BodyType defaults restored new scene',item('RadioButton','Dynamic')['value']=='true'and item('Checkbox','Enable')['value']=='true'and state()['hertz']==60 and state()['substeps']==4);screenshot('restored');m=metadata();r['restored_metadata']=m;recorded('old watched identities invalid after scene recreation',all(not x['valid']for x in m['watched']),{'metadata':m})
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_BODY_VIEW_WITH_FINDINGS'if r['findings']else 'PASS_BODY_VIEW_CONTROLS';save();emit('BodyType/View review closed findings='+str(len(r['findings'])))
'''
s=s[:a]+helpers+scenario+s[b:];compile(s,str(D/'body-view-controls-driver.py'),'exec');(D/'body-view-controls-driver.py').write_text(s);(D/'body-view-controls-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','sha256':hashlib.sha256(s.encode()).hexdigest(),'apps_CPUfirst':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'seconds_each':120,'ready_seconds':25,'positive_step_seconds':25,'screens_each':8,'actions_each':100,'retries':0},indent=2)+'\n')
s=(D/'run-catalog-controls.py').read_text().replace("'catalog-controls-matrix.json'","'body-view-controls-matrix.json'").replace("D/'catalog-controls-driver.py'","D/'body-view-controls-driver.py'").replace("raw/render-popup-build","raw/body-metadata-build").replace("'PASS_FIVE_ROUTES_FULL_CATALOG'","'REVIEWED_FIVE_ROUTES_BODY_VIEW'");compile(s,str(D/'run-body-view-controls.py'),'exec');(D/'run-body-view-controls.py').write_text(s)
print('bodyviewdriver syntax/protocol frozen')
