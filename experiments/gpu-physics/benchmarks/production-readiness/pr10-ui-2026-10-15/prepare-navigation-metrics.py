from pathlib import Path
import hashlib,json
D=Path(__file__).resolve().parent;o=(D/'menu-controls-clip-driver.py').read_text();helpers=o[o.index('\n def find(kind,field):'):o.index("\n open_menu('Sim');tap('MenuItem','Pause')")];s=(D/'ui-auto-driver.py').read_text().replace("D/('ui01-build-'+phase+'.json')","D/'render-popup-build.json'").replace("('ui01-'+phase+'-runs')","'navigation-metrics-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<120').replace("'180s overall automatic budget'","'120s navigationmetrics budget'").replace("len(r['actions'])<60","len(r['actions'])<100").replace("len(r['screenshots'])<9","len(r['screenshots'])<8").replace('until(state,90)','until(state,25)').replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');","env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');")
s=s.replace("p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'","p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'\n p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'")
a=s.index("\n key('p');until(");b=s.index('\nexcept BaseException',a)
scenario=r'''
 key('p');until(lambda:state()['pause']==1,3);r['findings']=[]
 def has_window(name):return any(x['name']==name for x in ui().get('windows',[]))
 def recorded_check(name,predicate,context):
  r['checks'][name]=bool(predicate)
  if not predicate:r['findings'].append({'check':name,'context':context})
  save();emit(('PASS 'if predicate else 'FINDING ')+name)
 def fresh_widget(kind,field):return until(lambda:(q if any(x['kind']==kind and x['label']==field for x in (q:=ui())['items'])else None),25)
 def reachable(kind,field):
  for attempt in range(8):
   q=frames();rows=[x for x in q['items']if x['kind']==kind and x['label']==field];assert rows,(kind,field);n=rows[-1]
   if n['visible']and n['rect'][1]>=n['clip'][1]and n['rect'][3]<=n['clip'][3]:return n
   hover((n['clip'][0]+n['clip'][2])/2,(n['clip'][1]+n['clip'][3])/2);direction=5 if(n['rect'][1]+n['rect'][3])/2>(n['clip'][1]+n['clip'][3])/2 else 4;action('scroll metrics to '+field,lambda:xdo('click','--repeat',2,'--delay',40,direction),n)
  raise AssertionError('metrics row not reachable '+field)
 def arrow(field):
  n=reachable('Tree',field);hover(n['rect'][0]+5,(n['rect'][1]+n['rect'][3])/2)
  def send():xdo('mousedown',1);held.add('mouse1');time.sleep(.04);xdo('mouseup',1);held.remove('mouse1')
  action('tree arrow '+field,send,n)
 key('m');until(lambda:state()['show_metrics']==1,3);tap('Tab','Profile');check('Profile tab selected',find('Tab','Profile')['value']=='true');screenshot('profile')
 tap('Checkbox','Show plots');check('plots enabled',find('Checkbox','Show plots')['value']=='true');arrow('solve');check('solve subtree opens',next(x for x in frames()['items']if x['kind']=='Tree'and x['label']=='solve')['value']=='true');arrow('constraints');check('constraints subtree opens',next(x for x in frames()['items']if x['kind']=='Tree'and x['label']=='constraints')['value']=='true');screenshot('plots');arrow('constraints');arrow('solve');check('solve subtree restored',next(x for x in frames()['items']if x['kind']=='Tree'and x['label']=='solve')['value']=='false');tap('Checkbox','Show plots');check('plots restored off',find('Checkbox','Show plots')['value']=='false')
 tap('Tab','Renderer');check('Renderer tab selected',find('Tab','Renderer')['value']=='true');q=frames();text=[x['label']for x in q['items']if x['kind'].startswith('Text')and x['window']=='Metrics'];r['renderer_text']=text;check('Renderer actual draw counters displayed',any(x.startswith('opaque ')for x in text)and any('draw calls'in x for x in text))
 tap('Tab','Frame Time');check('Frame Time tab selected',find('Tab','Frame Time')['value']=='true');screenshot('frame-time');tap('Tab','Profile');tap('Button','Reset');until(lambda:state()['sample_step']==0,3);check('Profile Reset resets step/history',state()['sample_step']==0);key('period');until(lambda:state()['sample_step']==1,3);open_menu('Sim');tap('MenuItem','Reset Profile');until(lambda:state()['sample_step']==0,3);check('menu Reset Profile resets step/history',state()['sample_step']==0);key('m');until(lambda:state()['show_metrics']==0,3)
 open_menu('Help');tap('MenuItem','Controls');until(lambda:has_window('Controls'),3);check('Help Controls visible',has_window('Controls'));screenshot('controls');key('Escape');until(lambda:not has_window('Controls'),3);check('Escape closes Controls',not has_window('Controls'));open_menu('Help');tap('MenuItem','About');until(lambda:has_window('About'),3);q=frames();check('About version visible',any(x['window']=='About'and x['label'].startswith('Box3D ')for x in q['items']));screenshot('about');open_menu('Help');tap('MenuItem','About');until(lambda:not has_window('About'),3);check('About closes',not has_window('About'))
 n=item('Header','Solver');click('Header','Solver');q=frames();check('Solver collapse hides controls',not any(x['label']=='Hertz##Solver'for x in q['items']));click('Header','Solver');q=frames();check('Solver reopen restores controls',any(x['label']=='Hertz##Solver'for x in q['items']))
 q=frames();hover(q['width']*.25,q['height']*.4);action('focus world outside GUI',lambda:xdo('click',1));old=state()['world'];key('r');q=frames()
 if any(w['name']=='Preparing simulation'for w in q.get('windows',[])):until(lambda:state()['world']!=old,25)
 else:
  end=time.monotonic()+.75
  while time.monotonic()<end and state()['world']==old:guard();time.sleep(.025)
 after=state();recorded_check('R restart from world focus',after['world']!=old,{'before_world':old,'after_state':after,'windows':q.get('windows')})
 q=frames();info=next(w for w in q['windows']if w['name']=='Info');hover(info['rect'][0]+20,info['rect'][1]+20);action('focus Info text',lambda:xdo('click',1));old=state()['world'];key('r');until(lambda:state()['world']!=old,25);until(lambda:has_window('Info'),25);check('R restart with Info focus',state()['world']!=old);check('restart keeps defaults',state()['hertz']==60 and state()['substeps']==4 and state()['context_flags']==[1,1,1]);screenshot('restored')
 open_menu('View');n=find('MenuItem','Hide UI');hover((n['rect'][0]+n['rect'][2])/2,(n['rect'][1]+n['rect'][3])/2);oldframe=state()['frame'];xdo('click',1);until(lambda:state()['frame']>oldframe+4,3);q=ui();recorded_check('Hide UI hides native windows',not any(w['name']in ['Info','##MainMenuBar']for w in q['windows']),{'actual_windows':q['windows'],'actual_world_state':state()})
 p=dest/'hidden.png';small(['ffmpeg','-hide_banner','-loglevel','error','-nostdin','-threads','1','-f','x11grab','-draw_mouse','0','-window_id',str(wid),'-i',env['DISPLAY'],'-frames:v','1','-threads','1',str(p)],5);r['screenshots']['hidden']={'path':str(p),'sha256':sha(p),'state':state(),'dimensions':list(struct.unpack('>II',p.read_bytes()[16:24]))};(dest/'hidden.ui.json').write_text(json.dumps(q,indent=2)+'\n');save();key('Tab');until(lambda:has_window('Info'),3);check('Tab restores UI',has_window('Info'))
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='REVIEWED_SHARED_NAVIGATION_METRICS_WITH_FINDINGS'if r['findings']else 'PASS_SHARED_NAVIGATION_METRICS';save();emit('Sharednavigation/metricsreviewclosed, findings='+str(len(r['findings'])))
'''
# Assignment expression must occur outside the comprehension iterable.
scenario=scenario.replace("lambda:(q if any(x['kind']==kind and x['label']==field for x in (q:=ui())['items'])else None)","lambda:(q if (q:=ui())and any(x['kind']==kind and x['label']==field for x in q['items'])else None)")
s=s[:a]+helpers+scenario+s[b:];compile(s,str(D/'navigation-metrics-driver.py'),'exec');(D/'navigation-metrics-driver.py').write_text(s);(D/'navigation-metrics-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','sha256':hashlib.sha256(s.encode()).hexdigest(),'apps_CPUfirst':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'seconds_each':120,'ready_seconds':25,'screens_each':8,'actions_each':100,'retries':0},indent=2)+'\n')
s=(D/'run-render-popup.py').read_text().replace("'render-popup-matrix.json'","'navigation-metrics-matrix.json'").replace("D/'render-popup-driver.py'","D/'navigation-metrics-driver.py'").replace("label+'-popup'","label").replace("'PASS_FIVE_ROUTES_RENDER_CONTROLS'","'REVIEWED_FIVE_ROUTES_NAVIGATION_METRICS'");compile(s,str(D/'run-navigation-metrics.py'),'exec');(D/'run-navigation-metrics.py').write_text(s)
print('navigationdriver syntax/protocol frozen')
