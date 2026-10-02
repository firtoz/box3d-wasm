from pathlib import Path
import hashlib,json,subprocess,os,time,signal,importlib.util
R=Path.cwd();A=R/'artifacts/production-readiness/pr01-current-acceptance';P=A/'protocol-before-work.json';p=json.loads(P.read_text());sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
def save():
 tmp=A/'receipt.tmp';tmp.write_text(json.dumps(state,indent=2)+'\n');os.replace(tmp,A/'receipt.json')
assert not (A/'receipt.json').exists();state={'status':'building','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'builds':[],'host_lists':[],'runs':[]};save()
def check_sources():
 for n,h in p['sources'].items():assert sha(R/n)==h,n
 b=json.loads(Path(p['engine_build_receipt']).read_text());assert sha(p['engine_build_receipt'])==p['engine_build_receipt_sha256'];assert b['inputs_before']==b['inputs_after']
 for n,h in b['inputs_before'].items():assert sha(R/n)==h,n
check_sources()
def run(cmd,out,env=None):
 out.mkdir(parents=True,exist_ok=False)
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(cmd,cwd=R,env=env,stdout=so,stderr=se,start_new_session=True);state['running']={'pid':proc.pid,'command':cmd,'result_directory':str(out)};save()
  try:code=proc.wait(timeout=p['watchdog_seconds_per_process'])
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);raise
 state.pop('running',None);save();return {'command':cmd,'exit':code,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log')}
def env(backend):
 e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=R,env=e);e.update(x.decode().split('=',1) for x in raw.split(b'\0') if b'=' in x)
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
planned=[('public-control',cell,trial) for cell in p['cells'] for trial in range(1,6)]+[('ccd-guard',k+'-gpu',1) for k in ['ordinary','native']]+[('state-capture',k,trial) for k in ['ordinary','native'] for trial in range(1,6)]+[('regression',k,1) for k in ['ordinary','native']]
try:
 for fixture,cells in [('speculative_fixture',list(p['cells'])),('speculative_ccd_guard_fixture',['ordinary-gpu','native-gpu'])]:
  for cell in cells:
   d=p['cells'][cell];build=Path(d['build']);both=cell.endswith('both');library=Path(d['library']);assert sha(library)==d['library_sha256'];receipt=json.loads(Path(d['receipt']).read_text());assert sha(d['receipt'])==d['receipt_sha256']
   linked=[library,build/'libgpu_samples_api.a',build/'box3d_src/libbox3d.a']+([build/'libgpu_both_api.a',build/'libbox3d_cpu.a'] if both else [])
   for f in linked:
    if f==library:assert sha(f)==d['library_sha256']
    elif str(f) in receipt['linked_archives']:assert sha(f)==receipt['linked_archives'][str(f)]
    else:assert f.exists(),f
   out=A/'links'/cell/fixture;binary=out/'fixture';cmd=['g++','-O2','-std=c++17',str(R/'c_abi'/(fixture+'.cpp')),'-I',str(R.parents[1]/'box3d/include'),'-I',str(R/'c_abi')]
   if both:cmd+=['-DGPU_API_DUAL','-Wl,--wrap=cpu_b3World_EnableSpeculative','-Wl,--whole-archive',str(build/'libgpu_both_api.a'),'-Wl,--no-whole-archive']
   cmd+=['-Wl,--whole-archive',str(build/'libgpu_samples_api.a'),'-Wl,--no-whole-archive',str(library)]
   if both:cmd.append(str(build/'libbox3d_cpu.a'))
   cmd +=[str(build/'box3d_src/libbox3d.a'),'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
   result=run(cmd,out);result.update(cell=cell,fixture=fixture,linked_inputs={str(f):sha(f) for f in linked},origin_viewer_receipt_sha256=d['receipt_sha256'],source_sha256=sha(R/'c_abi'/(fixture+'.cpp')),binary_sha256=sha(binary) if result['exit']==0 else None);state['builds'].append(result);save();assert result['exit']==0,result;check_sources()
 skip={}
 for backend,test in p['tests'].items():
  assert sha(test['path'])==test['sha256'];out=A/'host-list'/backend;row=run([test['path'],'--list'],out);row['backend']=backend;state['host_lists'].append(row);save();assert row['exit']==0
  listed=[l.rsplit(': test',1)[0] for l in (out/'stdout.log').read_text().splitlines() if l.endswith(': test')];assert set(p['regressions'])<=set(listed);skip[backend]=sorted(set(listed)-set(p['regressions']));(out/'selection.json').write_text(json.dumps({'selected':p['regressions'],'skipped':skip[backend],'count':63},indent=2)+'\n')
 state['status']='running';save()
 for kind,cell,trial in planned:
  backend=cell.split('-')[0];out=A/'runs'/kind/cell/str(trial);e=env(backend)
  if kind=='public-control':cmd=[str(A/'links'/cell/'speculative_fixture/fixture')]
  elif kind=='ccd-guard':cmd=[str(A/'links'/cell/'speculative_ccd_guard_fixture/fixture')]
  elif kind=='state-capture':
   out.parent.mkdir(parents=True,exist_ok=True);e['GPU_PHYSICS_TEST_TRACE']=str(out/'state.jsonl');cmd=[p['tests'][backend]['path'],'api::world::speculative_policy_tests::speculative_mesh_controls_capture_complete_step_state','--exact','--nocapture','--test-threads=1']
  else:
   cmd=[p['tests'][backend]['path'],'--nocapture','--test-threads=1']
   for name in skip[backend]:cmd+=['--skip',name]
  row=run(cmd,out,e);row.update(kind=kind,cell=cell,trial=trial,environment={k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_','VK_'))});state['runs'].append(row);save();assert row['exit']==0,row
  stdout=(out/'stdout.log').read_text();stderr=(out/'stderr.log').read_text();assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in stdout+stderr
  if kind=='public-control':assert 'speculative-controls=pass' in stdout
  elif kind=='ccd-guard':assert 'speculative-ccd-guard=pass' in stdout
  elif kind=='state-capture':
   assert '1 passed; 0 failed' in stdout
   spec=importlib.util.spec_from_file_location('state_health',R/'scripts/check-core-state-health.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);row['health']=m.check(out/'state.jsonl',110);row['trace_sha256']=sha(out/'state.jsonl');save()
  else:assert '63 passed; 0 failed' in stdout,stdout[-2000:]
  print(kind,cell,trial,'pass',flush=True)
 for cell in p['cells']:
  rows=[x for x in state['runs'] if x['kind']=='public-control' and x['cell']==cell];assert len(rows)==5 and len({x['stdout_sha256'] for x in rows})==1
 spec=importlib.util.spec_from_file_location('state_compare',R/'scripts/compare-core-state.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 for backend in p['tests']:
  paths=[A/'runs/state-capture'/backend/str(i)/'state.jsonl' for i in range(1,6)];result=m.compare(paths,110);(A/('state-comparison-'+backend+'.json')).write_text(json.dumps(result,indent=2)+'\n');assert result['status']=='pass',result
 check_sources();state.update(status='passed',finished_unix=time.time(),current_sources_unchanged=True);save()
except BaseException as ex:
 state.update(status='stopped',error=repr(ex),finished_unix=time.time(),unlaunched=[{'kind':k,'cell':c,'trial':t} for k,c,t in planned if not any(x['kind']==k and x['cell']==c and x['trial']==t for x in state['runs'])]);save();raise
print('PR01 current-source campaign complete; final release gates remain open',flush=True)
