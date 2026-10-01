from pathlib import Path
import os,json,hashlib,subprocess,signal,shlex,time
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-diagnostic';P=json.loads((A/'protocol-before-runs.json').read_text());base=Path(P['proof_directory']);sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();assert not(A/'receipt.json').exists();assert sha(R/P['fixture'])==P['fixture_sha256']and sha(base/'receipt.json')==P['proof_receipt_sha256'];old=json.loads((base/'receipt.json').read_text());assert old['status']=='built'and old['inputs_before']==old['inputs_after'];assert sha(R/'scripts/native-samples-cache-env.sh')==P['runtime_source_sha256']
D={'status':'building','pid':os.getpid(),'protocol_sha256':sha(A/'protocol-before-runs.json'),'fixture_sha256':P['fixture_sha256'],'builds':[],'runs':[],'proof':{},'started_unix':time.time(),'toolchain':subprocess.check_output(['g++','--version'],text=True),'adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version','--format=csv'],text=True)}
def save():(A/'receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def env(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
save()
try:
 binaries={}
 for cell in P['order']:
  folder=A/cell;folder.mkdir();v=json.loads((base/cell/'receipt.json').read_text());assert sha(base/cell/'receipt.json')==P['viewer_receipts'][cell];cm=base/cell/'cmake'
  if cell=='cpu':linked=[cm/'box3d_src/libbox3d.a'];recipe=['g++','-O2','-std=c++17','-ffp-contract=off',str(R/P['fixture']),'-I',str(R.parents[1]/'box3d/include')]
  else:
   backend,mode=cell.split('-');lib=Path(old['libraries'][backend]['path']);assert sha(lib)==old['libraries'][backend]['sha256'];cpu=base/(backend+'-both')/'cmake/libbox3d_cpu.a';cpu_receipt=json.loads((base/(backend+'-both')/'receipt.json').read_text());assert sha(cpu)==cpu_receipt['linked_archives'][str(cpu)]
   linked=([cm/'libgpu_both_api.a',cm/'libgpu_samples_api.a']if mode=='both'else[cm/'libgpu_samples_api.a'])+[lib,cpu,cm/'box3d_src/libbox3d.a'];recipe=['g++','-O2','-std=c++17','-ffp-contract=off','-DGPU_WORLD_DIAGNOSTIC',str(R/P['fixture']),'-I',str(R.parents[1]/'box3d/include')]
  for f in linked:
   if str(f)in v['linked_archives']:assert sha(f)==v['linked_archives'][str(f)]
  # Relevant direct C adapter/CPU units only; NFD/UI/observer are not linked here.
  units=[]
  for u in v['compiled_units']:
   obj=Path(u['object']);file=Path(u['file'])
   if file.suffix!='.c' or any(part in obj.parts for part in ['box3d_gfx','box3d_shared']) or not str(obj).startswith(str(cm)):continue
   if '/samples/'in str(file)or file.name in ['sokol_capacity.c']:continue
   if '/CMakeFiles/gpu_samples_api.dir/'not in str(obj) and '/CMakeFiles/gpu_both_api.dir/'not in str(obj) and '/box3d_src/'not in str(obj) and '/CMakeFiles/box3d_cpu_original.dir/'not in str(obj):continue
   assert sha(obj)==u['object_sha256']and sha(file)==u['source_sha256'];units.append(u)
  assert units
  D['proof'][cell]={'viewer_receipt_sha256':sha(base/cell/'receipt.json'),'linked_inputs':{str(f):sha(f)for f in linked},'applicable_compiled_units':units,'source_proof_report':'benchmarks/production-readiness/pr02-controls-current-builds-2026-10-01','scope_exclusion':'No NFD/UI/observer object in these C programs; existing engine source/dependency archive proof reused exactly.'};save()
  if cell=='cpu':recipe +=[str(linked[0])]
  else:
   count=2 if cell.endswith('both')else 1
   for f in linked[:count]:recipe+=['-Wl,--whole-archive',str(f),'-Wl,--no-whole-archive']
   recipe +=[str(f)for f in linked[count:]]+['-ldl','-lGL','-lgcc_s']
  binary=folder/'fixture';recipe+=['-lpthread','-lm','-o',str(binary)]
  with(folder/'build.log').open('w')as log:code=subprocess.run(recipe,stdout=log,stderr=subprocess.STDOUT).returncode
  D['builds'].append({'configuration':cell,'command':recipe,'exit':code,'log_sha256':sha(folder/'build.log')});save();assert code==0,(cell,code);D['builds'][-1]['binary_sha256']=sha(binary);binaries[cell]=binary;save();print(cell,'fixture built',flush=True)
 for cell in P['order']:
  folder=A/cell;binary=binaries[cell];e=env(cell.split('-')[0])if cell!='cpu'else{k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))};D['status']='running';save()
  with(folder/'stdout.jsonl').open('w')as out,(folder/'stderr.log').open('w')as err:
   proc=subprocess.Popen([str(binary)],stdout=out,stderr=err,env=e,start_new_session=True);D['running']={'configuration':cell,'pid':proc.pid};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  D.pop('running');rows=[json.loads(s)for s in(folder/'stdout.jsonl').read_text().splitlines()];summary=rows[-1]if rows else{};run={'configuration':cell,'exit':code,'binary_sha256':sha(binary),'stdout_sha256':sha(folder/'stdout.jsonl'),'stderr_sha256':sha(folder/'stderr.log'),'environment':{k:v for k,v in e.items()if k.startswith(('GPU_','WGPU_','VK_'))},'summary':summary};D['runs'].append(run);save();assert code==0 and summary.get('summary'),(cell,code);checks=[r for r in rows if 'match'in r];assert summary['observations']==len(checks)and summary['mismatches']==sum(not r['match']for r in checks);assert cell!='cpu'or summary['mismatches']==0;print(cell,summary,flush=True)
 D.update(status='diagnosed',finished_unix=time.time());D.pop('pid');save()
except BaseException as e:D.update(status='stopped',failure=repr(e),finished_unix=time.time());D.pop('pid',None);save();raise
