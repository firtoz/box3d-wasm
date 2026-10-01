from pathlib import Path
import hashlib,json,subprocess,tarfile,os,shlex,signal,math
ROOT=Path.cwd();A=ROOT/'artifacts/production-readiness/pr02-compound-properties-validation';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();P=json.loads((A/'protocol.json').read_text());assert not(A/'receipt.json').exists(),'Never rerun a closed or interrupted runner'
R={'status':'building','driver_pid':os.getpid(),'protocol_sha256':sha(A/'protocol.json'),'builds':[],'runs':[],'toolchain':{n:subprocess.check_output([n,'--version'],text=True)for n in ['cc','g++','cmake','ar']},'adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version,pci.bus_id','--format=csv'],text=True)}
def save():(A/'receipt.json').write_text(json.dumps(R,indent=2)+'\n')
def sources():
 fs=[p for p in (ROOT/'c_abi').glob('*')if p.is_file()]+[ROOT/n for n in ['scripts/gen-both-cpu.py','scripts/prepare-samples-link.py','native-samples/CMakeLists.txt']]+[p for f in ['src','include']for p in(ROOT.parents[1]/'box3d'/f).rglob('*')if p.is_file()and p.suffix in ['.c','.h']]
 return {os.path.relpath(f,ROOT):sha(f)for f in sorted(fs)}
def guard():
 assert sha(A/'protocol.json')==R['protocol_sha256']
 for n,h in P['production_candidate_sha256'].items():assert sha(ROOT/n)==h,n
 for n,h in P['fixture_sha256'].items():assert sha(ROOT/n)==h,n
 assert sha(ROOT/P['physical_validator'])==P['physical_validator_sha256']
 assert sha(ROOT/'scripts/native-samples-cache-env.sh')==P['runtime_environment_source_sha256']
 for b,e in P['engine_proof'].items():
  for n,h in [('receipt','receipt_sha256'),('library','library_sha256'),('tests','tests_sha256')]:assert sha(ROOT/e[n])==e[h]
  d=json.loads((ROOT/e['receipt']).read_text());assert d['status']=='built'and d['engine_sources']==d['engine_sources_after']
  for n,h in d['engine_sources'].items():assert sha(ROOT/n)==h,n
 if 'sources_before'in R:assert sources()==R['sources_before']
def archive(path,source_map,generated=()):
 with tarfile.open(path,'w:gz')as t:
  for n in source_map:t.add(ROOT/n,arcname=n.replace('../../box3d/','box3d/'))
  for f,rel in generated:t.add(f,arcname='generated/'+rel)
def build(cmd,log,**extra):
 guard()
 with log.open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 entry={'command':cmd,'exit':code,'log_path':str(log.relative_to(A)),'log_sha256':sha(log),**extra};R['builds'].append(entry);save();assert code==0,log;guard();return entry
def environment(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 if backend!='CPU':e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(ROOT/'artifacts/production-readiness/pr02-diagnostics/pipelines'))
 return e
markers={'native_diagnostic_populations':'native diagnostic populations:','api_settings_test':'C API settings: pass','both_compound_ownership_test':'combined compound ownership:','compound_aabb_contract':'compound AABB contract:'}
def mesh_check(cpu_file,gpu_file):
 cpu=[l.split()for l in cpu_file.read_text().splitlines()];gpu=[l.split()for l in gpu_file.read_text().splitlines()];assert len(cpu)==len(gpu)==48;worst_ray=worst_support=worst_mover=0
 for i,(a,b)in enumerate(zip(cpu,gpu)):
  assert all(math.isfinite(float(x))for x in a[1:]+b[1:])
  if i%6==5:
   assert a[:2]==b[:2]==['support',str(i//6)];ca,ga=float(a[2]),float(b[2]);assert .24<ca<.26 and .24<ga<.26;assert float(a[3])<.001 and float(b[3])<.001;worst_support=max(worst_support,abs(ca-ga));assert abs(ca-ga)<.0003,(a,b)
  elif i%6==4:
   assert a[:2]==b[:2]==['mover-cast',str(i//6)];ca,ga=float(a[2]),float(b[2]);assert .4<ca<.5 and .4<ga<.5;worst_mover=max(worst_mover,abs(ca-ga));assert abs(ca-ga)<1e-5,(a,b)
  elif i%6 in(2,3):assert a[:6]==b[:6]==['query'if i%6==2 else 'refit',str(i//6),'1','1','1','1'];assert abs(float(a[6])-float(b[6]))<1e-5,(a,b)
  else:
   assert a[:3]==b[:3]==[str(i//6),str(i%6),str(1-i%6)]
   for x,y in zip(a[3:7],b[3:7]):error=abs(float(x)-float(y));worst_ray=max(worst_ray,error);assert error<1e-5,(a,b)
   if i%6==0:assert a[7:]==b[7:]==['1','0']
 return {'status':'pass','rows':48,'maximum_ray_error':worst_ray,'maximum_support_error':worst_support,'maximum_mover_error':worst_mover}
try:
 guard();R['sources_before']=sources();save();archive(A/'fixture-source-inputs.tar.gz',R['sources_before'])
 adapter_dirs={};adapters={};binaries={}
 for cfg in P['configurations']:
  backend,mode=cfg.split('-');dest=A/cfg;dest.mkdir();cm=dest/'cmake';lib=ROOT/P['engine_proof'][backend]['library'];adapter_dirs[cfg]=cm
  build(['cmake','-S',str(ROOT/'native-samples'),'-B',str(cm),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_PORTABLE_API=ON','-DGPU_SAMPLES='+('ON'if mode=='gpu'else 'OFF'),'-DBOTH_SAMPLES='+('ON'if mode=='both'else 'OFF'),'-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release'],dest/'configure.log',configuration=cfg)
  build(['cmake','--build',str(cm),'--target']+(['gpu_both_api']if mode=='both'else [])+['gpu_samples_api','box3d','-j4'],dest/'adapter-build.log',configuration=cfg)
  linked=([cm/'libgpu_both_api.a']if mode=='both'else [])+[cm/'libgpu_samples_api.a',lib]+([cm/'libbox3d_cpu.a']if mode=='both'else [])+[cm/'box3d_src/libbox3d.a'];units=[]
  for u in json.loads((cm/'compile_commands.json').read_text()):
   tok=shlex.split(u['command']);obj=Path(u['directory'])/tok[tok.index('-o')+1]
   if obj.is_file()and str(obj).startswith(str(cm)):units.append({**u,'source_sha256':sha(u['file']),'object':str(obj),'object_sha256':sha(obj)})
  assert units
  generated=sorted(set(Path(u['file'])for u in units if str(u['file']).startswith(str(cm))))
  archive(dest/'compiled-adapter-inputs.tar.gz',R['sources_before'],[(f,str(f.relative_to(cm)))for f in generated]);receipt={'configuration':cfg,'sources_before':R['sources_before'],'sources_after':sources(),'compiled_units':units,'source_archive_sha256':sha(dest/'compiled-adapter-inputs.tar.gz'),'linked_inputs':{str(f):sha(f)for f in linked},'engine_proof':P['engine_proof'][backend]};(dest/'adapter-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');adapters[cfg]=receipt
 # Fresh complete CPU oracle generated in ordinary-both; standalone fixtures also need independently prefixed CPU for property reference.
 cpu_cm=adapter_dirs['ordinary-both'];cpu=cpu_cm/'libbox3d_unprefixed.a';prefix=cpu_cm/'libbox3d_cpu.a'
 for cfg in ['CPU']+P['configurations']:
  mode='CPU'if cfg=='CPU'else cfg.split('-')[1];backend='CPU'if cfg=='CPU'else cfg.split('-')[0];cm=None if cfg=='CPU'else adapter_dirs[cfg]
  names=['compound_property_diagnostic','compound_mesh_reference']if cfg=='CPU'else ['compound_property_diagnostic','compound_mesh_reference','native_diagnostic_populations','api_settings_test']+(['both_compound_ownership_test','compound_aabb_contract']if mode=='both'else [])
  for name in names:
   dest=A/cfg/name;dest.mkdir(parents=True);binary=dest/'fixture';cmd=['g++','-O2','-std=c++17','-ffp-contract=off',str(ROOT/'c_abi'/f'{name}.cpp'),'-I',str(ROOT.parents[1]/'box3d/include'),'-I',str(ROOT/'c_abi')]
   if cfg=='CPU':linked=[cpu];cmd += [str(cpu),'-lpthread','-lm']
   else:
    if name=='compound_property_diagnostic':cmd+=['-DGPU_PROPERTY_DIAGNOSTIC']
    if name=='compound_mesh_reference'and mode=='both':cmd+=['-DGPU_REFERENCE_BOTH']
    if name in ['native_diagnostic_populations','api_settings_test']:
     if name=='native_diagnostic_populations':cmd+=['-DGPU_POPULATIONS']
     if mode=='both':cmd+=['-DGPU_API_DUAL']
    linked=([cm/'libgpu_both_api.a']if mode=='both'else [])+[cm/'libgpu_samples_api.a',ROOT/P['engine_proof'][backend]['library']]+([cm/'libbox3d_cpu.a']if mode=='both'else ([prefix]if name=='compound_property_diagnostic'else []))+[cm/'box3d_src/libbox3d.a']
    count=2 if mode=='both'else 1
    for f in linked[:count]:cmd+=['-Wl,--whole-archive',str(f),'-Wl,--no-whole-archive']
    cmd += [str(f)for f in linked[count:]]+['-ldl','-lpthread','-lm','-lgcc_s','-lGL']
   cmd+=['-o',str(binary)];entry=build(cmd,dest/'build.log',configuration=cfg,fixture=name,linked_inputs={str(f):sha(f)for f in linked});entry['binary_sha256']=sha(binary);save();binaries[cfg,name]=binary
 assert len(binaries)==22,len(binaries)
 for b,e in P['engine_proof'].items():binaries[b+'-rust','compound_property_contract']=ROOT/e['tests']
 R.update(status='running',sources_after=sources());save();seen={};cpu_modes={}
 for index,case in enumerate(P['cases']):
  guard();cfg=case['configuration'];name=case['fixture'];binary=binaries[cfg,name];dest=A/cfg/name;dest.mkdir(parents=True,exist_ok=True);trial=seen.get((cfg,name),0)+1;seen[cfg,name]=trial;out=dest/f'{trial}-stdout.log';err=dest/f'{trial}-stderr.log';env=environment('CPU'if cfg=='CPU'else cfg.split('-')[0]);cmd=[str(binary)]+case['arguments']
  with out.open('w')as o,err.open('w')as e:
   proc=subprocess.Popen(cmd,env=env,stdout=o,stderr=e,start_new_session=True);R['running']={'index':index,'pid':proc.pid,**case,'trial':trial};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  R.pop('running');row={**case,'trial':trial,'command':cmd,'exit':code,'binary_sha256':sha(binary),'stdout_path':str(out.relative_to(A)),'stderr_path':str(err.relative_to(A)),'stdout_sha256':sha(out),'stderr_sha256':sha(err),'environment':{k:v for k,v in env.items()if k.startswith(('GPU_','WGPU_','VK_'))}};R['runs'].append(row);save();assert code==0,(case,code);guard()
  text=out.read_text()
  if name=='compound_property_diagnostic':
   rows=[json.loads(l)for l in text.splitlines()];summary=rows.pop();assert summary=={'summary':True,'observations':928 if cfg=='CPU'else 1016,'mismatches':0};assert len(rows)==summary['observations']and all(x['match']for x in rows);row['summary']=summary
  elif name=='compound_property_contract':assert '1 passed; 0 failed; 0 ignored;'in text;assert 'owned_table_and_generation_reuse_preserve_public_properties ... ok'in text;row['tests_passed']=1
  elif name=='compound_mesh_reference':
   if cfg=='CPU':cpu_modes[case['arguments'][0]]=out;row['mesh_validation']=mesh_check(out,out)
   else:row['mesh_validation']=mesh_check(cpu_modes[case['arguments'][0]],out)
  else:assert markers[name]in text
  if cfg!='CPU':assert 'NVIDIA GeForce RTX 4070 SUPER'in (err.read_text()+text)
  save();print(index+1,cfg,name,case['arguments'],'pass',flush=True)
 R.update(status='pass',sources_after=sources());guard();save()
except BaseException as e:
 R.update(status='stopped',error=str(e),unlaunched=P['cases'][len(R['runs']):],restoration={})
 for n,h in P['production_candidate_sha256'].items():
  if sha(ROOT/n)==h:(ROOT/n).write_bytes((A/'baseline'/n).read_bytes());R['restoration'][n]=sha(ROOT/n);assert R['restoration'][n]==P['baseline_production'][n]
  else:R['restoration'][n]='unexpected edits preserved'
 save();raise
finally:R.pop('driver_pid',None);R.pop('running',None);save()
print('First property validation complete:31 C+2 Rust processes pass. No timing or final release acceptance.',flush=True)
