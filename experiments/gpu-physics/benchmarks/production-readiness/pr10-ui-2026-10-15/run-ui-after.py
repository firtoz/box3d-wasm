from pathlib import Path
import json,subprocess,sys,time
D=Path(__file__).resolve().parent;build=json.loads((D/'ui01-build-after.json').read_text());assert build['status']=='PASS_UI_ONLY_CANDIDATE_BUILD';r={'status':'RUNNING','phases':[]};start=time.monotonic()
def save():(D/'ui01-after-state.json').write_text(json.dumps(r,indent=2)+'\n')
save()
try:
 for label in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
  before=json.loads((D/'raw/ui01-before-runs'/label/'receipt.json').read_text());assert before['visual_review']['status']=='PASS_NINE_SAVED_STAGES';print('AFTER START',label,flush=True);p=subprocess.Popen([sys.executable,str(D/'ui-auto-driver.py'),'after',label]);r.update(current_child_pid=p.pid,current_label=label);save();code=p.wait(timeout=195);r['phases'].append({'label':label,'driver_exit':code});save();assert code==0,label
 r['status']='NUMERIC_PASSES_VISUAL_AND_WRAP_AUDIT_PENDING'
except BaseException as e:r.update(status='FAILURE_RETAINED',error=repr(e));raise
finally:
 if 'p'in locals()and p.poll()is None:p.terminate();p.wait(timeout=6)
 r['elapsed_seconds']=time.monotonic()-start;save()
