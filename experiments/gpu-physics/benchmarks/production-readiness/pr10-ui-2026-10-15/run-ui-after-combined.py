from pathlib import Path
import json,subprocess,sys,time,hashlib
D=Path(__file__).resolve().parent;r={'status':'RUNNING','phases':[],'driver_sha256':hashlib.sha256((D/'ui-auto-driver.py').read_bytes()).hexdigest()};start=time.monotonic()
def save():(D/'ui01-after-combined-state.json').write_text(json.dumps(r,indent=2)+'\n')
save()
try:
 for label,tag in [('ordinary-both','ordinary-both-stat-safe'),('native-both','native-both')]:
  print('AFTER REMAINING START',label,flush=True);p=subprocess.Popen([sys.executable,str(D/'ui-auto-driver.py'),'after',label,tag]);r.update(current_child_pid=p.pid,current_label=label);save();code=p.wait(timeout=195);r['phases'].append({'label':label,'tag':tag,'driver_exit':code});save();assert code==0,label
 r['status']='NUMERIC_PASSES_VISUAL_AND_WRAP_AUDIT_PENDING'
except BaseException as e:r.update(status='FAILURE_RETAINED',error=repr(e));raise
finally:
 if 'p'in locals()and p.poll()is None:p.terminate();p.wait(timeout=6)
 r['elapsed_seconds']=time.monotonic()-start;save()
