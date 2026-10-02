from pathlib import Path
import sys,subprocess,json,time,os,signal,hashlib,re,gzip,shutil,resource
# This child recorder runs inside Xvfb before its wrapper performs cleanup.
if len(sys.argv)>1 and sys.argv[1]=='--child-exit':
 result=subprocess.run(sys.argv[3:]);Path(sys.argv[2]).write_text(json.dumps({'exit':result.returncode})+'\n');raise SystemExit(result.returncode if result.returncode>=0 else 128-result.returncode)
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-rain-drag-remaining';P=A/'protocol.json';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads(P.read_text());assert not(A/'receipt.json').exists();s={'status':'running','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'results':[]};resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check():
 for key in ['binary','helper']:assert sha(p['display_dependency'][key])==p['display_dependency'][key+'_sha256' if key=='helper' else 'sha256']
 for key in ['protocol','receipt']:assert sha(p['previous_attempt'][key+'_path'])==p['previous_attempt'][key+'_sha256']
 for f,h in p['producer_receipts'].items():
  assert sha(f)==h;d=json.loads(Path(f).read_text());assert d['inputs_before']==d['inputs_after']
  for n,h in d['inputs_after'].items():assert sha(R/n)==h,n
 for n,h in p['source_hashes'].items():assert sha(R/n)==h
 assert sha(p['fixture_link_receipt'])==p['fixture_link_receipt_sha256']
 for c in p['cases']:
  d=c.get('binary',c.get('fixture'));assert sha(d['binary'])==d['binary_sha256']
  if 'receipt' in d:assert sha(d['receipt'])==d['receipt_sha256']
  if 'fixture' in c:
   for n,h in d['linked_inputs'].items():assert sha(n)==h,n
save()
try:
 check()
 for c in p['cases']:
  out=A/c['name'];out.mkdir();cmd=c['command'];cwd=Path(c['cwd']);start=time.time()
  if c['kind']=='rain':
   cwd.mkdir();(cwd/c['initial_settings']['name']).write_text(c['initial_settings']['bytes']);cmd=[sys.executable,p['display_dependency']['helper'],p['display_dependency']['binary'],str(out/'child-exit.json'),*cmd]
  with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
   proc=subprocess.Popen(cmd,cwd=cwd,env=c['environment'],stdout=so,stderr=se,start_new_session=True);s['running']={'name':c['name'],'pid':proc.pid,'wrapper_command':cmd};save()
   try:code=proc.wait(timeout=p['watchdog_seconds_per_process'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
  s.pop('running',None);err=(out/'stderr.log').read_text();stdout=(out/'stdout.log').read_text();adapter=c['cell']=='cpu' or ('NVIDIA GeForce RTX 4070 SUPER' in err and ('Vulkan' in err or 'backend=Vulkan' in err));row={'name':c['name'],'kind':c['kind'],'wrapper_exit':code,'elapsed_seconds_incidental':time.time()-start,'actual_NVIDIA_Vulkan':adapter if c['cell']!='cpu' else None,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log')}
  if c['kind']=='rain':
   row['child_exit']=json.loads((out/'child-exit.json').read_text())['exit'] if (out/'child-exit.json').exists() else None
   row['health_exists']=(out/'health.json').exists()
   if row['health_exists']:row['health_sha256']=sha(out/'health.json');row['health_bytes']=(out/'health.json').stat().st_size
   row['sokol_errors']=re.findall(r'samples: (\d+) frames, (\d+) sokol errors',err)
  elif c['kind']=='drag':
   data=out/'comparison.txt';row['paired_trace_sha256']=sha(data) if data.exists() else None;row['trace_last_frame']=max((int(m.group(1)) for m in re.finditer(r'^F (\d+) ',data.read_text(),re.M)),default=-1) if data.exists() else -1;row['summary_lines']=[l for l in stdout.splitlines() if l.startswith(('PASS','FAIL','ground motion:'))]
  else:
   match=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed;',stdout);row['harness_result']=match.groups() if match else None
  s['results'].append(row);save();print(json.dumps(row),flush=True);check();assert code!='timeout' and adapter,row
 s.update(status='completed-baseline',finished_unix=time.time());save()
except BaseException as ex:s.update(status='stopped',error=repr(ex),finished_unix=time.time(),unlaunched=[c['name'] for c in p['cases'][len(s['results']):]]);save();raise
