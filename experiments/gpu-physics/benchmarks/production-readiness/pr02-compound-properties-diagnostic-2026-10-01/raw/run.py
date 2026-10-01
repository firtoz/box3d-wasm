from pathlib import Path
import hashlib,json,subprocess,tarfile,os,shlex,signal
R=Path.cwd();A=R/'artifacts/production-readiness/pr02-compound-properties-diagnostic';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();P=json.loads((A/'protocol.json').read_text());assert sha(R/P['fixture'])==P['fixture_sha256'];assert not(A/'receipt.json').exists()
D={'status':'building','protocol_sha256':sha(A/'protocol.json'),'driver_pid':os.getpid(),'builds':[],'runs':[],'toolchain':{n:subprocess.check_output([n,'--version'],text=True)for n in ['cc','g++','cmake','ar']},'adapter':subprocess.check_output(['nvidia-smi','--query-gpu=name,uuid,driver_version,pci.bus_id','--format=csv'],text=True)}
def save():(A/'receipt.json').write_text(json.dumps(D,indent=2)+'\n')
def sources():
 fs=[p for p in (R/'c_abi').glob('*')if p.is_file()]+[R/n for n in ['scripts/gen-both-cpu.py','scripts/prepare-samples-link.py','native-samples/CMakeLists.txt']]+[p for f in ['include','src']for p in(R.parents[1]/'box3d'/f).rglob('*')if p.is_file()and p.suffix in ['.c','.h']]
 return{os.path.relpath(p,R):sha(p)for p in sorted(fs)}
def build(cmd,log,**extra):
 with log.open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 d={'command':cmd,'exit':code,'log_sha256':sha(log),**extra};D['builds'].append(d);save();assert code==0,log;return d
def archive(path,files):
 with tarfile.open(path,'w:gz')as t:
  for n,p in files:t.add(p,arcname=n)
def environment(backend):
 e={k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
 if backend=='native':
  raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'],env=e);e.update(x.decode().split('=',1)for x in raw.split(b'\0')if x.startswith((b'GPU_',b'WGPU_')))
 e.update(GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(R/'artifacts/production-readiness/pr02-diagnostics/pipelines'))
 return e
try:
 before=sources();D['sources_before']=before;save();assert sha(R/'scripts/native-samples-cache-env.sh')==P['runtime_environment_source_sha256']
 for b,proof in P['engine_proof'].items():
  assert sha(R/proof['library'])==proof['library_sha256']and sha(R/proof['receipt'])==proof['receipt_sha256'];d=json.loads((R/proof['receipt']).read_text());assert d['engine_sources']==d['engine_sources_after']
  for n,h in d['engine_sources'].items():assert sha(R/n)==h,n
 archive(A/'fixture-source-inputs.tar.gz',[(n.replace('../../box3d/','box3d/'),R/n)for n in before])
 binaries={}
 cpu=R/P['combined_proof']['ordinary']['directory']/'libbox3d_unprefixed.a'
 cfg=A/'CPU';cfg.mkdir();binary=cfg/'fixture';entry=build(['g++','-O2','-std=c++17','-ffp-contract=off',str(R/P['fixture']),'-I',str(R.parents[1]/'box3d/include'),str(cpu),'-lpthread','-lm','-o',str(binary)],cfg/'build.log',configuration='CPU',linked_inputs={str(cpu):sha(cpu)})
 entry['binary_sha256']=sha(binary);binaries['CPU']=binary;save()
 for b in ['ordinary','native']:
  lib=R/P['engine_proof'][b]['library'];cfg=A/(b+'-gpu');cfg.mkdir();cm=cfg/'cmake';build(['cmake','-S',str(R/'native-samples'),'-B',str(cm),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_PORTABLE_API=ON','-DGPU_SAMPLES=ON','-DBOTH_SAMPLES=OFF','-DFETCHCONTENT_FULLY_DISCONNECTED=ON','-DCMAKE_BUILD_TYPE=Release'],cfg/'configure.log',configuration=b+'-gpu');build(['cmake','--build',str(cm),'--target','gpu_samples_api','box3d','-j4'],cfg/'adapter-build.log',configuration=b+'-gpu')
  assert sources()==before
  units=[]
  for u in json.loads((cm/'compile_commands.json').read_text()):
   tok=shlex.split(u['command']);obj=Path(u['directory'])/tok[tok.index('-o')+1]
   if obj.is_file()and str(obj).startswith(str(cm)):units.append({**u,'source_sha256':sha(u['file']),'object':str(obj),'object_sha256':sha(obj)})
  generated=[(str(Path(u['file']).relative_to(cm)),Path(u['file']))for u in units if str(u['file']).startswith(str(cm))]
  archive(cfg/'compiled-adapter-inputs.tar.gz',[(n.replace('../../box3d/','box3d/'),R/n)for n in before]+[('generated/'+n,f)for n,f in generated]);adapter={'sources_before':before,'sources_after':sources(),'compiled_units':units,'source_archive_sha256':sha(cfg/'compiled-adapter-inputs.tar.gz')};(cfg/'adapter-receipt.json').write_text(json.dumps(adapter,indent=2)+'\n')
  for mode in ['gpu','both']:
   dest=A/(b+'-'+mode);dest.mkdir(exist_ok=True);binary=dest/'fixture'
   bothcm=R/P['combined_proof'][b]['directory'];proof=R/P['combined_proof'][b]['receipt'];assert sha(proof)==P['combined_proof'][b]['receipt_sha256'];old=json.loads(proof.read_text())
   for u in old['compiled_units']:
    if str(u['object']).startswith(str(bothcm)):assert sha(u['file'])==u['source_sha256']and sha(u['object'])==u['object_sha256']
   assert old['sources_before']==old['sources_after']
   # Newly added fixture is not a compiled adapter unit. Reuse exact source/object/archive proof.
   linked=[cm/'libgpu_samples_api.a',lib,bothcm/'libbox3d_cpu.a',cm/'box3d_src/libbox3d.a']if mode=='gpu'else[bothcm/'libgpu_both_api.a',bothcm/'libgpu_samples_api.a',lib,bothcm/'libbox3d_cpu.a',bothcm/'box3d_src/libbox3d.a']
   for f in linked:
    if str(f)in old['linked_inputs']:assert sha(f)==old['linked_inputs'][str(f)]
   cmd=['g++','-O2','-std=c++17','-ffp-contract=off','-DGPU_PROPERTY_DIAGNOSTIC',str(R/P['fixture']),'-I',str(R.parents[1]/'box3d/include'),'-I',str(R/'c_abi')]
   for f in linked[:2]if mode=='both'else linked[:1]:cmd+=['-Wl,--whole-archive',str(f),'-Wl,--no-whole-archive']
   cmd +=[str(f)for f in (linked[2:]if mode=='both'else linked[1:])]+['-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)]
   entry=build(cmd,dest/'build.log',configuration=b+'-'+mode,linked_inputs={str(f):sha(f)for f in linked});entry['binary_sha256']=sha(binary);binaries[b+'-'+mode]=binary;save()
 assert sources()==before
 for cfg in P['order']:
  dest=A/cfg;binary=binaries[cfg];env=environment(cfg.split('-')[0])if cfg!='CPU'else {k:v for k,v in os.environ.items()if not k.startswith(('GPU_','WGPU_','VK_'))}
  D.update(status='running');save()
  with(dest/'stdout.jsonl').open('w')as out,(dest/'stderr.log').open('w')as err:
   proc=subprocess.Popen([str(binary)],stdout=out,stderr=err,env=env,start_new_session=True);D['running']={'configuration':cfg,'pid':proc.pid};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  run={'configuration':cfg,'exit':code,'binary_sha256':sha(binary),'stdout_sha256':sha(dest/'stdout.jsonl'),'stderr_sha256':sha(dest/'stderr.log'),'environment':{k:v for k,v in env.items()if k.startswith(('GPU_','WGPU_','VK_'))}};D['runs'].append(run);D.pop('running',None);save();assert code==0,(cfg,code)
  rows=[json.loads(x)for x in(dest/'stdout.jsonl').read_text().splitlines()];summary=rows[-1];assert summary.get('summary')and summary['observations']==len(rows)-1;assert summary['mismatches']==sum(not x['match']for x in rows[:-1]);run.update(summary=summary);save();assert cfg!='CPU'or summary['mismatches']==0
  print(cfg,'complete',summary,flush=True)
 D.update(status='diagnosed',sources_after=sources());assert D['sources_after']==before;save()
except BaseException as e:
 D.update(status='stopped',error=str(e),unlaunched=[n for n in P['order']if not any(x['configuration']==n for x in D['runs'])]);save();raise
finally:
 D.pop('driver_pid',None);D.pop('running',None);save()
