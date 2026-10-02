from pathlib import Path
import sys,os,subprocess,json,time,hashlib,shutil,re,signal,resource,tarfile
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-sleeper-fixture';P=A/'protocol.json';sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();p=json.loads(P.read_text());assert not(A/'receipt.json').exists();previous=json.loads(Path(p['run_after_terminal_campaign']).read_text());assert previous['status'] in ['stopped','completed-baseline'] and not previous.get('running');resource.setrlimit(resource.RLIMIT_CORE,(0,0));s={'status':'starting','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'prior_campaign_receipt_sha256':sha(p['run_after_terminal_campaign']),'source_revision_context':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'started_unix':time.time(),'builds':[],'tests':[],'inputs_before':p['original_inputs'],'toolchain':subprocess.check_output(['rustc','-Vv'],text=True)}
def save():
 temp=A/'receipt.tmp';temp.write_text(json.dumps(s,indent=2)+'\n');os.replace(temp,A/'receipt.json')
def check(inputs):
 for n,h in inputs.items():assert sha(R/n)==h,n
 for d in [p['ordinary_library'],*p['libraries'].values()]:assert sha(d['path'])==d['sha256'],d['path']
def execute(command,out,env,timeout):
 out.mkdir();start=time.time()
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(command,stdout=so,stderr=se,env=env,start_new_session=True);s['running']={'pid':proc.pid,'command':command};save()
  try:code=proc.wait(timeout=timeout)
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None);return {'exit':code,'command':command,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'elapsed_seconds_incidental':time.time()-start}
save();edited=False
try:
 check(p['original_inputs']);assert p['test_source_cfg_gate']['text'] in (R/'src/lib.rs').read_text();assert sha(R/'src/lib.rs')==p['test_source_cfg_gate']['src/lib.rs_sha256'];shutil.copyfile(A/'candidate-gpu_invariants.rs',R/'src/gpu_invariants.rs');edited=True;check(p['candidate_inputs']);s.update(status='building',inputs_after_edit=p['candidate_inputs']);save()
 with tarfile.open(A/'candidate-source-inputs.tar.gz','w:gz') as t:
  for n in p['candidate_inputs']:t.add(R/n,arcname=n,recursive=False)
 for backend,command in p['recipes']:
  check(p['candidate_inputs']);out=A/(backend+'-build');row=execute(command,out,os.environ.copy(),p['build_watchdog_seconds']);row['backend']=backend;s['builds'].append(row);save();assert row['exit']==0,row
  records=[]
  for line in (out/'stdout.log').read_text().splitlines():
   try:records.append(json.loads(line))
   except ValueError:pass
  artifacts=[v for v in records if v.get('reason')=='compiler-artifact' and v['target']['name']=='gpu_physics' and v.get('executable')];assert len(artifacts)==1;artifact=artifacts[0];exe=A/(backend+'-test-executable');shutil.copyfile(artifact['executable'],exe);row.update(executable=str(exe),executable_sha256=sha(exe),compiler_artifact=artifact);s['builds'][-1]=row;check(p['candidate_inputs']);save();print(backend,'testbuildcomplete',flush=True)
 s['status']='checking';save()
 for c in p['cases']:
  check(p['candidate_inputs']);built=next(v for v in s['builds'] if v['backend']==c['backend']);assert sha(built['executable'])==built['executable_sha256'];out=A/c['name'];row=execute([built['executable'],c['selector'],'--exact','--nocapture','--test-threads=1'],out,c['environment'],p['test_watchdog_seconds']);row['name']=c['name'];stdout=(out/'stdout.log').read_text();stderr=(out/'stderr.log').read_text();row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in stderr and 'Vulkan' in stderr;match=re.search(r'test result: (ok|FAILED)\. (\d+) passed; (\d+) failed; (\d+) ignored;',stdout);row['harness_result']=list(match.groups()) if match else None;row['fixture_lines']=[line for line in stderr.splitlines() if line.startswith('sleeper fixture:')];row['pass']=row['exit']==0 and row['harness_result']==['ok','1','0','0'] and len(row['fixture_lines'])==1;s['tests'].append(row);save();print(json.dumps(row),flush=True);check(p['candidate_inputs']);assert row['exit']!='timeout' and row['actual_NVIDIA_Vulkan'] and row['harness_result'],row
 s.update(status='completed-fixture-checks',finished_unix=time.time(),inputs_after={n:sha(R/n) for n in p['candidate_inputs']},all_assertions_pass=all(t['pass'] for t in s['tests']));save()
except BaseException as e:
 if edited and (len(s['builds'])<2 or (s['builds'] and s['builds'][-1]['exit']!=0)):
  shutil.copyfile(A/'original-gpu_invariants.rs',R/'src/gpu_invariants.rs');s['test_source_restored']=True
 s.update(status='stopped',error=repr(e),finished_unix=time.time());s.pop('running',None);save();raise
