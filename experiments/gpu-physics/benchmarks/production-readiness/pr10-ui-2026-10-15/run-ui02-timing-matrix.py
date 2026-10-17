from pathlib import Path
import subprocess,json,time
D=Path(__file__).resolve().parent;r={'status':'RUNNING','runs':[]};start=time.monotonic()
def save():(D/'ui02-timing-matrix.json').write_text(json.dumps(r,indent=2)+'\n')
try:
 for phase,labels in [('after',['native-gpu','ordinary-both','native-both'])]:
  for label in labels:
   cmd=['python3',str(D/'ui02-after-timing-driver.py'),phase,label,label+'-timing-observed'];t=time.monotonic();p=subprocess.Popen(cmd,start_new_session=True);r['live']={'phase':phase,'label':label,'pid':p.pid,'command':cmd};save();code=p.wait(timeout=70);r['runs'].append({'phase':phase,'label':label,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,(phase,label,code)
 r['status']='PASS_ALL_UI02_BEFORE_AFTER_ROUTES'
except BaseException as e:
 r['status']='FAILURE_RETAINED';r['error']=repr(e)
 if 'p'in globals()and p.poll()is None:
  import os,signal
  os.killpg(p.pid,signal.SIGTERM)
  try:p.wait(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGKILL);p.wait(timeout=5)
 raise
finally:r.pop('live',None);r['seconds']=time.monotonic()-start;save()
