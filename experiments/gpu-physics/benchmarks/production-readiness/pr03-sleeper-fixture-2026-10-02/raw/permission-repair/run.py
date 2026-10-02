from pathlib import Path
import os,json,hashlib,subprocess,time,signal,re,resource,stat
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-sleeper-launch-repair';P=A/'protocol.json';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads(P.read_text());assert not(A/'receipt.json').exists();s={'status':'starting','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'permissions':[],'tests':[]};resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check():
 for key in ['protocol','receipt']:assert sha(p['prior_'+key])==p['prior_'+key+'_sha256']
 for n,h in p['candidate_inputs'].items():assert sha(R/n)==h,n
 for d in p['libraries']+list(p['binaries'].values()):assert sha(d['path'])==d['sha256']
save()
try:
 check()
 for backend,d in p['binaries'].items():
  f=Path(d['path']);assert stat.S_IMODE(f.stat().st_mode)==d['mode_before'];f.chmod(d['mode_after']);assert sha(f)==d['sha256'];s['permissions'].append({'backend':backend,'path':str(f),'mode_before':d['mode_before'],'mode_after':stat.S_IMODE(f.stat().st_mode),'binary_sha256_unchanged':sha(f)});save()
 s['status']='checking';save()
 for c in p['cases']:
  check();d=p['binaries'][c['backend']];out=A/c['name'];out.mkdir();cmd=[d['path'],c['selector'],'--exact','--nocapture','--test-threads=1'];start=time.time()
  with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
   proc=subprocess.Popen(cmd,env=c['environment'],stdout=so,stderr=se,start_new_session=True);s['running']={'pid':proc.pid,'command':cmd};save()
   try:code=proc.wait(timeout=p['test_watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
  s.pop('running',None);text=(out/'stdout.log').read_text();err=(out/'stderr.log').read_text();m=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed; (\d+) ignored;',text);row={'name':c['name'],'command':cmd,'exit':code,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'actual_NVIDIA_Vulkan':'NVIDIA GeForce RTX 4070 SUPER' in err and 'Vulkan' in err,'harness_result':list(m.groups()) if m else None,'fixture_lines':[l for l in err.splitlines() if l.startswith('sleeper fixture:')],'elapsed_seconds_incidental':time.time()-start};row['pass']=code==0 and row['harness_result']==['ok','1','0','0'] and len(row['fixture_lines'])==1;s['tests'].append(row);save();print(json.dumps(row),flush=True);check();assert code!='timeout' and row['actual_NVIDIA_Vulkan'] and m,row
 s.update(status='completed-fixture-checks',all_assertions_pass=all(v['pass'] for v in s['tests']),finished_unix=time.time());save()
except BaseException as e:s.update(status='stopped',error=repr(e),finished_unix=time.time());s.pop('running',None);save();raise
