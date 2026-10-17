from pathlib import Path
import subprocess,time,json,os,signal
D=Path(__file__).resolve().parent;r={'status':'RUNNING','runs':[]};start=time.monotonic()
def save():(D/'body-view-transition-matrix.json').write_text(json.dumps(r,indent=2)+'\n')
try:
 for label in ['ordinary-gpu','native-gpu','ordinary-both','native-both']:
  cmd=['python3',str(D/('body-view-ordinary-tail.py'if label=='ordinary-gpu'else 'body-view-transition-driver.py')),'after',label,'transition-'+label];log=(D/'raw/body-metadata-build'/(label+'-driver.log')).open('w');p=subprocess.Popen(cmd,start_new_session=True,stdout=log,stderr=subprocess.STDOUT);print('START '+label,flush=True);r['live']={'label':label,'pid':p.pid,'command':cmd};save();t=time.monotonic();code=p.wait(timeout=250);log.close();print('CLOSED '+label+' exit='+str(code)+' seconds='+str(round(time.monotonic()-t,3)),flush=True);r['runs'].append({'label':label,'exit':code,'seconds':time.monotonic()-t});save();r.setdefault('failed_routes',[]).extend([label]if code else [])
 r['status']='REVIEWED_FIVE_ROUTES_BODY_VIEW'
except BaseException as e:
 r['status']='FAILURE_RETAINED';r['error']=repr(e)
 if 'p'in globals()and p.poll()is None:
  os.killpg(p.pid,signal.SIGTERM)
  try:p.wait(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGKILL);p.wait(timeout=5)
 raise
finally:r.pop('live',None);r['seconds']=time.monotonic()-start;save()
