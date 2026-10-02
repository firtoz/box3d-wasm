from pathlib import Path
import json,hashlib,subprocess,time,os,signal,re,resource
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-distance-baseline';P=A/'protocol.json';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads(P.read_text());assert not(A/'receipt.json').exists();s={'status':'running','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'results':[]};resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check(c):
 assert sha(p['build_receipt'])==p['build_receipt_sha256'];f=c['fixture'];assert sha(f['binary'])==f['binary_sha256'];assert sha(f['source'])==f['source_sha256']
 for n,h in f['linked_inputs'].items():assert sha(n)==h
save()
try:
 for c in p['cases']:
  check(c);out=A/c['name'];out.mkdir();start=time.time()
  with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
   proc=subprocess.Popen(c['command'],env=c['environment'],cwd=R,stdout=so,stderr=se,start_new_session=True);s['running']={'name':c['name'],'pid':proc.pid};save()
   try:code=proc.wait(timeout=p['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
  s.pop('running',None);err=(out/'stderr.log').read_text();adapter='NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err;match=re.search(r'(sphere-ground|distance-joint) step=(\d+) lane=(\d+) GPU=([^ ]+) CPU=([^\s]+)',err)
  row={'name':c['name'],'exit':code,'elapsed_seconds_incidental':time.time()-start,'actual_NVIDIA_Vulkan':adapter,'first_discrepancy':match.groups() if match else None,'completed_fixture':code==0,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log')};s['results'].append(row);save();check(c);print(json.dumps(row),flush=True);assert adapter and code!='timeout',row
 s.update(status='completed-baseline',finished_unix=time.time());save()
except BaseException as ex:s.update(status='stopped',error=repr(ex),finished_unix=time.time());save();raise
