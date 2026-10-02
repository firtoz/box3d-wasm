from pathlib import Path
import json,hashlib,subprocess,os,time,signal,shutil,gzip,tarfile,resource
R=Path.cwd();A=R/'artifacts/production-readiness/pr04-drag-ccd-boundary';P=A/'protocol.json';p=json.loads(P.read_text());assert not (A/'receipt.json').exists()
sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();resource.setrlimit(resource.RLIMIT_CORE,(0,0))
s={'status':'preflight','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'toolchain':subprocess.check_output(['cc','--version'],text=True),'commands':[],'results':[],'binaries':{}}
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check():
 for n,h in p['inputs'].items():assert sha(n)==h,n
 for n,h in p['CPP_consumer_inputs'].items():assert sha(R/n)==h,n
 for n,h in p['compiled_Rust_inputs_current'].items():assert sha(R/n)==h,n
 for u in p['compiled_units']:assert sha(u['file'])==u['source_sha256'] and sha(u['object'])==u['object_sha256']
 for link in p['links']:
  for n,h in link['reused_inputs'].items():assert sha(n)==h,n
 for c in p['cases']:assert sha(c['baseline_trace'])==c['baseline_trace_sha256']
def execute(cmd,name,cwd,env=None):
 d=A/name;d.mkdir();start=time.time()
 with (d/'stdout.log').open('w') as so,(d/'stderr.log').open('w') as se:
  proc=subprocess.Popen(cmd,cwd=cwd,env=env,stdout=so,stderr=se,start_new_session=True);s['running']={'name':name,'pid':proc.pid,'command':cmd};save()
  try:code=proc.wait(timeout=p['watchdog_seconds'])
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None)
 return {'name':name,'command':cmd,'cwd':str(cwd),'exit':code,'stdout_sha256':sha(d/'stdout.log'),'stderr_sha256':sha(d/'stderr.log'),'elapsed_seconds_incidental':time.time()-start}
def prefix(data):
 lines=[]
 for line in data.decode().splitlines():
  if line.startswith('B ') and int(line.split()[1])>=240:break
  if line.startswith(('B ','F ','M ','P ')):lines.append(line)
 assert sum(x.startswith('B ') for x in lines)==1200
 assert sum(x.startswith('F ') for x in lines)==2400
 return lines
save()
try:
 check();s['status']='building';save()
 row=execute(p['compile_command'],'CPU-observer-compile',p['compile_cwd']);s['commands'].append(row);save();assert row['exit']==0,row
 s['CPU_observer_object_sha256']=sha(A/'solver.c.o')
 # Hash every actual compiler dependency, including system headers. Archive those
 # bytes separately so an invocation source list is not the only compilation proof.
 dep=(A/'solver-dependencies.d').read_text().replace('\\\n',' ').split(':',1)[1].split()
 s['CPU_compile_dependencies']={str(Path(n).resolve() if Path(n).is_absolute() else (Path(p['compile_cwd'])/n).resolve()):sha(Path(n) if Path(n).is_absolute() else Path(p['compile_cwd'])/n) for n in dep}
 with tarfile.open(A/'CPU-compile-dependencies.tar.gz','w:gz') as t:
  for n in s['CPU_compile_dependencies']:t.add(n,arcname=n.lstrip('/'),recursive=False)
 shutil.copyfile(p['CPU_archive'],A/'cpu-observer-unprefixed.a');s['CPU_archive_copy_sha256']=sha(A/'cpu-observer-unprefixed.a');assert s['CPU_archive_copy_sha256']==p['inputs'][p['CPU_archive']]
 row=execute(['ar','r',str(A/'cpu-observer-unprefixed.a'),str(A/'solver.c.o')],'CPU-archive-replace',R);s['commands'].append(row);save();assert row['exit']==0
 members=subprocess.check_output(['ar','t',str(A/'cpu-observer-unprefixed.a')],text=True).splitlines();assert set(members)==set(p['CPU_archive_members'])
 s['CPU_observer_archive_members']={n:hashlib.sha256(subprocess.check_output(['ar','p',str(A/'cpu-observer-unprefixed.a'),n])).hexdigest() for n in members}
 assert [n for n,h in s['CPU_observer_archive_members'].items() if h!=p['CPU_archive_members'][n]]==['solver.c.o']
 row=execute(['objcopy','--redefine-syms='+p['symbol_map'],str(A/'cpu-observer-unprefixed.a'),str(A/'libbox3d_cpu_observer.a')],'CPU-archive-prefix',R);s['commands'].append(row);save();assert row['exit']==0
 s['CPU_observer_archive_sha256']=sha(A/'libbox3d_cpu_observer.a')
 for link in p['links']:
  check();row=execute(link['command'],link['name']+'-link',R);s['commands'].append(row);save();assert row['exit']==0,row
  s['binaries'][link['name']]={'path':link['command'][-1],'sha256':sha(link['command'][-1]),'Rust_library_sha256':link['Rust_library_sha256']};save()
 s['status']='capturing';save()
 for c in p['cases']:
  check();assert sha(c['command'][0])==s['binaries'][c['name']]['sha256']
  env={k:v for k,v in os.environ.items() if not k.startswith(('GPU_PHYSICS_','WGPU_','VK_','BOTH_DRAG_'))};env.update(c['environment_overrides'])
  row=execute(c['command'],c['name'],R,env);row['environment_overrides']=c['environment_overrides'];s['results'].append(row);save()
  stdout=(A/c['name']/'stdout.log').read_text();stderr=(A/c['name']/'stderr.log').read_text()
  row['actual_NVIDIA_Vulkan']='NVIDIA GeForce RTX 4070 SUPER' in stderr and 'backend=Vulkan' in stderr
  row['completed_prefix']='DIAGNOSTIC completed 240-step original drag prefix' in stdout
  trace=A/c['name']/'comparison.txt';assert trace.exists();raw=trace.read_bytes();expected=prefix(gzip.decompress(Path(c['baseline_trace']).read_bytes()));actual=prefix(raw)
  row['observer_neutral']=actual==expected;row['comparison_sha256']=sha(trace);row['baseline_prefix_sha256']=hashlib.sha256(('\n'.join(expected)+'\n').encode()).hexdigest();row['observer_prefix_sha256']=hashlib.sha256(('\n'.join(actual)+'\n').encode()).hexdigest();row['body_state_records']=2400;row['comparison_records']=len(actual)
  if actual!=expected:
   row['first_prefix_difference']=next(({'line':i,'expected':a,'actual':b} for i,(a,b) in enumerate(zip(expected,actual)) if a!=b),{'expected_lines':len(expected),'actual_lines':len(actual)})
  row['pass']=row['exit']==0 and row['actual_NVIDIA_Vulkan'] and row['completed_prefix'] and row['observer_neutral'];save();print(json.dumps({k:row[k] for k in ['name','exit','completed_prefix','observer_neutral','pass']}),flush=True);assert row['pass'],row
  with (A/c['name']/'comparison.txt.gz').open('wb') as f:f.write(gzip.compress(raw,mtime=0))
 check();s.update(status='completed-boundary-diagnostic',finished_unix=time.time(),all_observer_checks_pass=all(x['pass'] for x in s['results']));save()
except BaseException as e:
 s.update(status='stopped',error=repr(e),finished_unix=time.time(),unlaunched=[c['name'] for c in p['cases'] if not any(x['name']==c['name'] for x in s['results'])]);save();raise
