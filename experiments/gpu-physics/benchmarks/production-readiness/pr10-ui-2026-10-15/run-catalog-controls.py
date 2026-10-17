from pathlib import Path
import subprocess,time,json,os,signal
D=Path(__file__).resolve().parent;r={'status':'RUNNING','runs':[]};start=time.monotonic()
def save():(D/'catalog-controls-matrix.json').write_text(json.dumps(r,indent=2)+'\n')
try:
 for label in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
  cmd=['python3',str(D/'catalog-controls-driver.py'),'after',label,label];log=(D/'raw/render-popup-build'/(label+'-driver.log')).open('w');p=subprocess.Popen(cmd,start_new_session=True,stdout=log,stderr=subprocess.STDOUT);print('START '+label,flush=True);r['live']={'label':label,'pid':p.pid,'command':cmd};save();t=time.monotonic();code=p.wait(timeout=130);log.close();print('CLOSED '+label+' exit='+str(code)+' seconds='+str(round(time.monotonic()-t,3)),flush=True);r['runs'].append({'label':label,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,(label,code)
 r['status']='PASS_FIVE_ROUTES_FULL_CATALOG'
except BaseException as e:
 r['status']='FAILURE_RETAINED';r['error']=repr(e)
 if 'p'in globals()and p.poll()is None:
  os.killpg(p.pid,signal.SIGTERM)
  try:p.wait(timeout=5)
  except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGKILL);p.wait(timeout=5)
 raise
finally:r.pop('live',None);r['seconds']=time.monotonic()-start;save()
