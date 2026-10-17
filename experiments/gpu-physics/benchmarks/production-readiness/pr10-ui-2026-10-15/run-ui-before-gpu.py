from pathlib import Path
import json,subprocess,sys,time
D=Path(__file__).resolve().parent;start=time.monotonic();cpu=json.loads((D/'raw/ui01-before-runs/cpu/receipt.json').read_text());assert cpu['visual_review']['status']=='PASS_NINE_SAVED_STAGES';r={'status':'RUNNING','phases':[],'started':time.time()}
def save():(D/'ui01-before-gpu-state.json').write_text(json.dumps(r,indent=2)+'\n')
save()
try:
 for label in ['ordinary-gpu','native-gpu','ordinary-both','native-both']:
  print('BEFORE START',label,flush=True);p=subprocess.Popen([sys.executable,str(D/'ui-auto-driver.py'),'before',label]);r['current_child_pid']=p.pid;r['current_label']=label;save();code=p.wait(timeout=195);r['phases'].append({'label':label,'driver_exit':code});save();assert code==0,label
 r['status']='NUMERIC_PASSES_VISUAL_REVIEW_PENDING'
except BaseException as e:
 r.update(status='FAILURE_RETAINED',error=repr(e));raise
finally:
 if 'p'in locals()and p.poll()is None:p.terminate();p.wait(timeout=6)
 r['elapsed_seconds']=time.monotonic()-start;save()
