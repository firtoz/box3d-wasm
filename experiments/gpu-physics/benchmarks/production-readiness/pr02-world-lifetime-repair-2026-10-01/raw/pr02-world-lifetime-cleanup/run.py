from pathlib import Path
import os,json,hashlib,subprocess,signal,shlex,time,shutil
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-world-lifetime-cleanup';B=R/'artifacts/production-readiness/pr02-controls-current-builds';F=R/'artifacts/production-readiness/pr02-world-lifetime-fix';N=R/'artifacts/production-readiness/pr02-world-lifetime-artifact-repair';V=R/'artifacts/production-readiness/pr02-world-lifetime-rust-validation';H=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();P=json.loads((A/'protocol-before-change.json').read_text());C=json.loads((A/'candidate-inputs.json').read_text());assert not(A/'receipt.json').exists();vr=json.loads((V/'receipt.json').read_text());assert vr['status']=='passed'and len(vr['runs'])==18;assert all(H(R/f)==h for f,h in C.items());old=json.loads((B/'receipt.json').read_text());native=json.loads((N/'build-receipt.json').read_text());ordinary=json.loads((N/'ordinary-recovered.json').read_text());libs={'ordinary':ordinary['existing_library'],'native':native['libraries']['native']};assert all(H(x['path'])==x['sha256']for x in libs.values());D={'status':'building','pid':os.getpid(),'protocol_sha256':H(A/'protocol-before-change.json'),'Rust_validation_sha256':H(V/'receipt.json'),'Rust_build_sha256':H(N/'build-receipt.json'),'ordinary_recovery_sha256':H(N/'ordinary-recovered.json'),'candidate_C':C,'libraries':libs,'builds':[],'generators':[],'unit_compiles':[],'proof':{},'runs':[],'started':time.time(),'adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version','--format=csv'],text=True),'compiler':subprocess.check_output(['cc','--version'],text=True)}
def save():(A/'receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def command(cmd,log,cwd=R):
 with log.open('w')as f:code=subprocess.run(cmd,cwd=cwd,stdout=f,stderr=subprocess.STDOUT).returncode
 assert code==0,(cmd,code)
def env(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(A/'pipelines'));return e
save()
try:
 order=['ordinary-gpu','native-gpu','ordinary-both','native-both'];archives={}
 for cell in order:
  folder=A/cell;folder.mkdir();backend,mode=cell.split('-');cm=B/cell/'cmake';v=json.loads((B/cell/'receipt.json').read_text());gen=['python3','scripts/prepare-samples-link.py','--root',str(R),'--box3d',str(R.parents[1]/'box3d'),'--rust-lib',libs[backend]['path'],'--out',str(folder/'api-sources'),'--nm','nm']+(['--both']if mode=='both'else[]);command(gen,folder/'generator.log');D['generators'].append({'configuration':cell,'command':gen,'log_sha256':H(folder/'generator.log')});save()
  # All unchanged generated C is compared byte-exact to original compiled source.
  checked={}
  for f in (cm/'api-sources').glob('*.c'):
   nextfile=folder/'api-sources'/f.name
   if f.name=='shim.c'and mode=='gpu':continue
   assert H(f)==H(nextfile),(cell,f.name);checked[f.name]=H(f)
  relevant=[]
  for u in v['compiled_units']:
   if '/CMakeFiles/gpu_samples_api.dir/'in u['object'] or '/CMakeFiles/gpu_both_api.dir/'in u['object']:
    assert H(u['object'])==u['object_sha256'];relevant.append(u)
  D['proof'][cell]={'original_viewer_receipt_sha256':H(B/cell/'receipt.json'),'unchanged_generated_C':checked,'original_adapter_units':relevant,'archive_replacements':[]};save()
  linked=[]
  for target in (['gpu_both_api','gpu_samples_api']if mode=='both'else['gpu_samples_api']):
   src=cm/('lib'+target+'.a');assert H(src)==v['linked_archives'][str(src)];dst=folder/src.name;shutil.copy2(src,dst)
   changed='both_dual.c'if target=='gpu_both_api'else('shim.c'if mode=='gpu'else None)
   if changed:
    units=[u for u in relevant if Path(u['file']).name==changed];assert len(units)==1;u=units[0];newsource=R/'c_abi/both_dual.c'if changed=='both_dual.c'else folder/'api-sources/shim.c';obj=folder/(changed+'.o');cmd=shlex.split(u['command']);cmd[cmd.index('-o')+1]=str(obj);cmd[cmd.index('-c')+1]=str(newsource);command(cmd,folder/(changed+'.build.log'),cwd=cm);D['unit_compiles'].append({'configuration':cell,'command':cmd,'source':str(newsource),'source_sha256':H(newsource),'object_sha256':H(obj),'log_sha256':H(folder/(changed+'.build.log'))});save();command(['ar','r',str(dst),str(obj)],folder/(changed+'.archive.log'));D['proof'][cell]['archive_replacements'].append({'archive':str(dst),'baseline_sha256':H(src),'new_sha256':H(dst),'member':obj.name,'object_sha256':H(obj)});save()
   linked.append(dst)
  cpu=B/(backend+'-both')/'cmake/libbox3d_cpu.a';cpu_proof=json.loads((B/(backend+'-both')/'receipt.json').read_text());assert H(cpu)==cpu_proof['linked_archives'][str(cpu)];filtered=cm/'box3d_src/libbox3d.a';assert H(filtered)==v['linked_archives'][str(filtered)];archives[cell]=linked+[Path(libs[backend]['path']),cpu,filtered];D['proof'][cell]['linked_inputs']={str(p):H(p)for p in archives[cell]};save()
 assert len(D['unit_compiles'])==4
 binaries={};recipes=[('cpu','cleanup')]+[(x,'cleanup')for x in order]+[(x,'root')for x in order]
 for cell,fixture in recipes:
  folder=A/cell;folder.mkdir(exist_ok=True);src=R/'c_abi'/('world_cleanup_contract.cpp'if fixture=='cleanup'else'world_lifetime_diagnostic.cpp');cmd=['g++','-O2','-std=c++17','-ffp-contract=off',str(src),'-I',str(R.parents[1]/'box3d/include')]
  if cell=='cpu':
   lib=B/'cpu/cmake/box3d_src/libbox3d.a';v=json.loads((B/'cpu/receipt.json').read_text());assert H(lib)==v['linked_archives'][str(lib)];cmd+=[str(lib)];D['proof']['cpu']={'linked_archive_sha256':H(lib),'original_viewer_receipt_sha256':H(B/'cpu/receipt.json')}
  else:
   cmd+=['-DGPU_CLEANUP_CONTRACT'if fixture=='cleanup'else'-DGPU_WORLD_DIAGNOSTIC'];linked=archives[cell];count=2 if cell.endswith('both')else 1
   for lib in linked[:count]:cmd+=['-Wl,--whole-archive',str(lib),'-Wl,--no-whole-archive']
   cmd +=[str(f)for f in linked[count:]]+['-ldl','-lGL','-lgcc_s']
  binary=folder/(fixture+'-executable');cmd+=['-lpthread','-lm','-o',str(binary)];log=folder/(fixture+'.build.log');command(cmd,log);D['builds'].append({'configuration':cell,'fixture':fixture,'command':cmd,'log_sha256':H(log),'binary_sha256':H(binary)});binaries[cell,fixture]=binary;save();print('built',cell,fixture,flush=True)
 for cell,fixture in recipes:
  folder=A/cell;binary=binaries[cell,fixture];e=env(cell.split('-')[0])if cell!='cpu'else{k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))};D['status']='running';save()
  with(folder/(fixture+'.stdout.jsonl')).open('w')as out,(folder/(fixture+'.stderr.log')).open('w')as err:
   proc=subprocess.Popen([str(binary)],stdout=out,stderr=err,env=e,start_new_session=True);D['running']={'pid':proc.pid,'configuration':cell,'fixture':fixture};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  D.pop('running');rows=[json.loads(l)for l in(folder/(fixture+'.stdout.jsonl')).read_text().splitlines()];summary=rows[-1]if rows else{};result={'configuration':cell,'fixture':fixture,'exit':code,'binary_sha256':H(binary),'environment':{k:v for k,v in e.items()if k.startswith(('GPU_','WGPU_','VK_'))},'stdout_sha256':H(folder/(fixture+'.stdout.jsonl')),'stderr_sha256':H(folder/(fixture+'.stderr.log')),'summary':summary};D['runs'].append(result);save();assert code==0 and summary.get('summary')and summary.get('mismatches')==0;checks=[x for x in rows if'match'in x];assert summary['observations']==len(checks)>0 and all(x['match']for x in checks);assert all(H(R/f)==h for f,h in C.items());print('passed',cell,fixture,summary['observations'],flush=True)
 D.update(status='passed',finished=time.time());D.pop('pid');save()
except BaseException as e:
 D.update(status='stopped',failure=repr(e),finished=time.time());D.pop('pid',None);save()
 for f,h in P['baseline_C'].items():assert H(A/'baseline'/f)==h;shutil.copy2(A/'baseline'/f,R/f)
 root=json.loads((F/'protocol-before-change.json').read_text())
 for f,h in root['baseline'].items():assert H(F/'baseline'/f)==h;shutil.copy2(F/'baseline'/f,R/f)
 (R/'src/api/world/world_lifetime.rs').unlink(missing_ok=True);D['production_restored']=True;save();raise
