from pathlib import Path
import json,hashlib
D=Path(__file__).resolve().parent;original=(D/'menu-controls-clip-driver.py').read_text();helpers=original[original.index('\n def find(kind,field):'):original.index("\n open_menu('Sim');tap('MenuItem','Pause')")];s=(D/'ui-auto-driver.py').read_text().replace("D/('ui01-build-'+phase+'.json')","D/'ui03-output-build.json'").replace("('ui01-'+phase+'-runs')","'ui03-before-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<60').replace("'180s overall automatic budget'","'60s UI03 output budget'").replace("len(r['actions'])<60","len(r['actions'])<30").replace("len(r['screenshots'])<9","len(r['screenshots'])<4").replace('until(state,90)','until(state,25)').replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');","env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_API_TRACE']=str(dest/'api.jsonl');")
s=s.replace("p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'","p=dest/'observer.jsonl';assert not p.exists()or p.stat().st_size<16*1024**2,'Observer cap'\n p=dest/'api.jsonl';assert not p.exists()or p.stat().st_size<8*1024**2,'API cap'")
a=s.index("\n key('p');until(");b=s.index('\nexcept BaseException',a)
scenario=r'''
 key('p');until(lambda:state()['pause']==1,3)
 def all_api():
  p=dest/'api.jsonl';return [json.loads(x)for x in p.read_text().splitlines()]if p.exists()else []
 r['output_cases']=[];initial=draw();check('initial Bounds/Mass disabled',initial['bounds']==0 and initial['mass']==0);screenshot('default')
 for field,flag,output,stage in [('Bounds','bounds','bounds_callbacks','bounds'),('Mass','mass','transform_callbacks','mass')]:
  index=len(all_api());open_menu('View');tap('MenuItem',field);proof=draw_value(flag,1);frames();rows=all_api()[index:];outputs=[x for x in rows if x['api']=='actual_draw_outputs'];gpu=[x for x in rows if x['api']=='actual_gpu_draw_outputs'];assert outputs;values=[x[output]for x in outputs];enabled_rows=[x for x in rows if x['api']=='b3World_Draw'and x[flag]==1];assert enabled_rows
  # Use the final output pair(s), after the actual flag transition completed.
  values=[x[output]for x in outputs[-4:]];gpu_values=[x[output]for x in gpu[-4:]]
  check(field+' input reaches drawing',proof[flag]==1)
  if label=='cpu':check(field+' CPU emits callback',max(values)>0)
  elif 'both'in label:
   check(field+' combined CPU emits callback',max(values)>0);check(field+' combined GPU missing callback demonstrated',bool(gpu_values)and max(gpu_values)==0)
  else:check(field+' GPU missing callback demonstrated',max(values)==0)
  r['output_cases'].append({'control':field,'actual_draw_flags':proof,'callback_outputs':outputs[-8:],'actual_gpu_outputs':gpu[-8:],'all_recorded_output_count':len(outputs),'scene_state':state()});screenshot(stage)
  open_menu('View');tap('MenuItem',field);proof=draw_value(flag,0);check(field+' restored',proof[flag]==0)
 screenshot('restored');focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='UI03_ACTUAL_OUTPUT_DEFECT_CONFIRMED'if label!='cpu'else 'UI03_CPU_OUTPUT_REFERENCE_VERIFIED';save();emit('UI03 actual callback output confirmation complete; no repair')
'''
s=s[:a]+helpers+scenario+s[b:];compile(s,str(D/'ui03-output-driver.py'),'exec');(D/'ui03-output-driver.py').write_text(s);(D/'ui03-output-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','sha256':hashlib.sha256(s.encode()).hexdigest(),'apps_CPUfirst':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'seconds_each':60,'ready_seconds':25,'screens_each':4,'actions_each':30,'retries':0,'scope':'ActualBounds/Mass callbackoutput andoriginalscreens. Sharedsourceotheroverlaysunimplemented remainexplicitnotruntimequalified.'},indent=2)+'\n')
s=(D/'run-menu-controls-clip.py').read_text().replace("'menu-controls-clip-matrix.json'","'ui03-output-matrix.json'").replace("D/'menu-controls-clip-driver.py'","D/'ui03-output-driver.py'").replace("raw/menu-controls-clip-build","raw/ui03-output-build").replace("'after',label,label+'-clip'","'before',label").replace('p.wait(timeout=130)','p.wait(timeout=70)').replace("'PASS_FIVE_ROUTES_SIM_VIEW_SUBSET'","'UI03_OUTPUT_CONFIRMED_FIVE_ROUTES'");compile(s,str(D/'run-ui03-output.py'),'exec');(D/'run-ui03-output.py').write_text(s)
print('UI03 outputdriver syntax/protocol frozen')
