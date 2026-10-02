from pathlib import Path
import hashlib,json,subprocess,os,time,signal
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-current-acceptance';P=A/'protocol-before-work.json';p=json.loads(P.read_text());sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest()
def write(f,d):
 t=f.with_suffix(f.suffix+'.tmp');t.write_text(json.dumps(d,indent=2)+'\n');os.replace(t,f)
assert not (A/'receipt.json').exists()
state={'status':'building','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'builds':[],'C_runs':[],'Rust_runs':[],'budget':p['budget']}
def save():write(A/'receipt.json',state)
save()
parent=Path(p['cells']['ordinary-gpu']['receipt']).parent.parent/'receipt.json'
parent_hash=sha(parent)
def check_sources():
 for n,h in p['inputs'].items():assert sha(R/n)==h,n
 assert sha(parent)==parent_hash
 for receiptpath,expectedhash in [(Path(p['engine_build_receipt']),p['engine_build_receipt_sha256']),(parent,parent_hash)]:
  assert sha(receiptpath)==expectedhash
  b=json.loads(receiptpath.read_text());assert b['status'] in ('built','passed','pass');assert b['inputs_before']==b['inputs_after']
  for n,h in b['inputs_before'].items():assert sha(R/n)==h,n
 for test in p['tests'].values():assert sha(test['path'])==test['sha256']
 for backend,record in p['reuse_current_host_lists'].items():
  assert sha(record['path'])==record['sha256'];listed=Path(record['path']).read_text();assert all(x+': test' in listed for x in p['Rust_selectors'])
def run(cmd,out,env=None):
 out.mkdir(parents=True,exist_ok=False);start=time.time()
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(cmd,cwd=R,env=env,stdout=so,stderr=se,start_new_session=True);state['running']={'pid':proc.pid,'command':cmd,'result_directory':str(out)};save()
  try:code=proc.wait(timeout=p['watchdog_seconds'])
  except subprocess.TimeoutExpired:
   os.killpg(proc.pid,signal.SIGTERM)
   try:proc.wait(timeout=30)
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait()
   code='timeout'
 state.pop('running',None);save();return {'command':cmd,'exit':code,'started_unix':start,'finished_unix':time.time(),'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log')}
def env(backend):
 e={k:v for k,v in os.environ.items() if not k.startswith(('GPU_','WGPU_','VK_','__NV','__GLX'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],cwd=R,env=e);e.update(x.decode().split('=',1) for x in raw.split(b'\0') if b'=' in x)
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
markers={'api_settings_test':'C API settings: pass','warm_start_fixture':'"timing_only":false','native_diagnostics_fixture':'native diagnostics C contract passed:','native_diagnostic_populations':'native diagnostic populations: sensor/compound/mesh contract passed','both_substep_forces_test':'substep forces:','both_shape_replacement_test':'shape replacement:','both_joint_reaction_test':'joint reaction:','both_joint_separation_test':'joint separation:','both_compound_ownership_test':'combined compound ownership: four child kinds, bounds, public constructors and three lifetime cycles passed','compound_aabb_contract':'compound AABB contract: public/native/body/CPU geometry, transforms, disabled access and stale lifetimes passed'}
try:
 check_sources();state['viewer_parent_receipt_sha256']=parent_hash
 for case in p['cases']:
  cell=case['cell'];fixture=case['fixture'];d=p['cells'][cell];build=Path(d['build']);both=cell.endswith('both');library=Path(d['library']);assert sha(library)==d['library_sha256'];receipt=json.loads(Path(d['receipt']).read_text());assert sha(d['receipt'])==d['receipt_sha256']
  linked=[library,build/'libgpu_samples_api.a',build/'box3d_src/libbox3d.a']+([build/'libgpu_both_api.a'] if both else [])
  cpu=None
  if both or case['independent_prefixed_CPU']:cpu=Path(p['cells'][cell.split('-')[0]+'-both']['build'])/'libbox3d_cpu.a';linked.append(cpu)
  combinedreceipt=json.loads(Path(p['cells'][cell.split('-')[0]+'-both']['receipt']).read_text());assert sha(p['cells'][cell.split('-')[0]+'-both']['receipt'])==p['cells'][cell.split('-')[0]+'-both']['receipt_sha256']
  for f in linked:
   expected=d['library_sha256'] if f==library else (combinedreceipt if f==cpu else receipt)['linked_archives'][str(f)];assert sha(f)==expected,f
  out=A/'links'/cell/fixture;binary=out/'fixture';cmd=['g++','-O2','-std=c++17',str(R/'c_abi'/(fixture+'.cpp')),'-I',str(R.parents[1]/'box3d/include'),'-I',str(R/'c_abi')]+case['flags']
  if both:cmd+=['-DGPU_API_DUAL','-Wl,--whole-archive',str(build/'libgpu_both_api.a'),'-Wl,--no-whole-archive']
  cmd+=['-Wl,--whole-archive',str(build/'libgpu_samples_api.a'),'-Wl,--no-whole-archive',str(library)]
  if cpu:cmd.append(str(cpu))
  cmd +=[str(build/'box3d_src/libbox3d.a'),'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
  row=run(cmd,out);row.update(cell=cell,fixture=fixture,linked_inputs={str(f):sha(f) for f in linked},origin_viewer_receipt_sha256=d['receipt_sha256'],source_sha256=sha(R/'c_abi'/(fixture+'.cpp')),binary_sha256=sha(binary) if row['exit']==0 else None);state['builds'].append(row);save();assert row['exit']==0,row;check_sources();print('linked',cell,fixture,flush=True)
 state['status']='running';save()
 for case in p['cases']:
  cell=case['cell'];fixture=case['fixture'];check_sources();binary=A/'links'/cell/fixture/'fixture';buildrow=next(x for x in state['builds'] if x['cell']==cell and x['fixture']==fixture);assert sha(binary)==buildrow['binary_sha256']
  for f,h in buildrow['linked_inputs'].items():assert sha(f)==h
  e=env(cell.split('-')[0]);out=A/'runs/C'/cell/fixture;row=run([str(binary)],out,e);row.update(cell=cell,fixture=fixture,binary_sha256=sha(binary),environment={k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_','VK_'))});state['C_runs'].append(row);save();assert row['exit']==0,row
  stdout=(out/'stdout.log').read_text();stderr=(out/'stderr.log').read_text();assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in stdout+stderr
  if fixture=='compound_property_diagnostic':
   records=[json.loads(s) for s in stdout.splitlines() if s.startswith('{')];summary=records[-1];assert summary=={'summary':True,'observations':1016,'mismatches':0},summary;assert all(x['match'] for x in records[:-1]);row['property_summary']=summary;save()
  else:assert markers[fixture] in stdout,(fixture,stdout[-2000:])
  print('C passed',cell,fixture,flush=True)
 for i,selector in enumerate(p['Rust_selectors'],1):
  for backend in ['ordinary','native']:
   check_sources();test=p['tests'][backend];assert sha(test['path'])==test['sha256'];e=env(backend);out=A/'runs/Rust'/backend/str(i);row=run([test['path'],selector,'--exact','--nocapture','--test-threads=1'],out,e);row.update(backend=backend,selector=selector,binary_sha256=sha(test['path']),host_only=selector==p['host_only_selector'],environment={k:v for k,v in e.items() if k.startswith(('GPU_','WGPU_','VK_'))});state['Rust_runs'].append(row);save();assert row['exit']==0,row
   stdout=(out/'stdout.log').read_text();stderr=(out/'stderr.log').read_text();assert '1 passed; 0 failed' in stdout,stdout[-2000:]
   if not row['host_only']:assert 'NVIDIA GeForce RTX 4070 SUPER | backend=Vulkan' in stdout+stderr
   print('Rust passed',backend,selector,flush=True)
 check_sources();assert len(state['builds'])==32 and len(state['C_runs'])==32 and len(state['Rust_runs'])==20;state.update(status='passed',finished_unix=time.time(),current_sources_unchanged=True);save()
except BaseException as ex:
 state.update(status='stopped',error=repr(ex),finished_unix=time.time(),unlaunched_C=[c for c in p['cases'] if not any(x['cell']==c['cell'] and x['fixture']==c['fixture'] for x in state['C_runs'])],unlaunched_Rust=[{'backend':b,'selector':s} for s in p['Rust_selectors'] for b in ['ordinary','native'] if not any(x['backend']==b and x['selector']==s for x in state['Rust_runs'])]);save();raise
print('PR02 current contract acceptance passed; broader release gates remain open',flush=True)
