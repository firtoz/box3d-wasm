from pathlib import Path
import hashlib,json,subprocess,tarfile,os,re,shutil,signal,math
ROOT=Path.cwd();A=ROOT/'artifacts/production-readiness/pr02-compound-properties-lifetime';OLD=ROOT/'artifacts/production-readiness/pr02-compound-properties-validation';COMP=ROOT/'artifacts/production-readiness/pr02-compound-properties-compile';sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();P=json.loads((A/'protocol-before-correction.json').read_text());C=json.loads((A/'candidate-inputs.json').read_text());assert not(A/'receipt.json').exists();R={'status':'building','driver_pid':os.getpid(),'protocol_sha256':sha(A/'protocol-before-correction.json'),'candidate_inputs_sha256':sha(A/'candidate-inputs.json'),'builds':[],'runs':[],'applicability':'Exact same production libraries and C fixture binaries; only cfg(test) module differs. No property processes repeated.'}
def save():(A/'receipt.json').write_text(json.dumps(R,indent=2)+'\n')
def guard():
 assert sha(A/'protocol-before-correction.json')==C['protocol_sha256']==R['protocol_sha256']
 assert sha(OLD/'protocol.json')==P['prior_validation_protocol_sha256']and sha(OLD/'receipt.json')==P['prior_validation_receipt_sha256']
 for n,h in C['production_candidate_sha256'].items():assert sha(ROOT/n)==h,n
 marker='#[cfg(test)]\nmod compound_property_contract {';before=(COMP/'candidate/src/api/world.rs').read_text();after=(ROOT/'src/api/world.rs').read_text();assert before.count(marker)==after.count(marker)==1;assert before.split(marker)[0]==after.split(marker)[0],'Non-test production source changed'
 for b,e in P['engine_proof'].items():
  for n,h in [('receipt','receipt_sha256'),('library','library_sha256')]:assert sha(ROOT/e[n])==e[h]
  d=json.loads((ROOT/e['receipt']).read_text());assert d['engine_sources']==d['engine_sources_after']
  for n,h in d['engine_sources'].items():
   if n!='src/api/world.rs':assert sha(ROOT/n)==h,n
 for x in old['builds']:
  if 'binary_sha256'in x:
   assert sha(x['command'][-1])==x['binary_sha256']
   for n,h in x['linked_inputs'].items():assert sha(n)==h,n
 for n,h in old['sources_before'].items():assert sha(ROOT/n)==h,n
old=json.loads((OLD/'receipt.json').read_text());assert old['status']=='stopped'and len(old['runs'])==10 and old['runs'][-1]['exit']==101
cases=[{'configuration':b+'-rust','fixture':'compound_property_contract','arguments':['compound_property_contract','--test-threads=1','--nocapture']}for b in ['ordinary','native']]+P['remaining_C_cases'];assert len(cases)==24
save()
try:
 guard();binaries={}
 for b,e in P['engine_proof'].items():
  dest=A/b;dest.mkdir();proof=json.loads((ROOT/e['receipt']).read_text());names=proof['engine_sources'];before={n:sha(ROOT/n)for n in names}
  with tarfile.open(dest/'test-compiled-sources.tar.gz','w:gz')as t:
   for n in names:t.add(ROOT/n,arcname=n.replace('../../box3d/','box3d/'))
  cmd=['cargo','test','--release','--lib','--features','external-c-shim,replay-diagnostics','--target-dir','target/samples','--no-run']if b=='ordinary'else ['bash','scripts/build-native-cache.sh','test','--release','--lib','--features','external-c-shim,replay-diagnostics','--no-run'];log=dest/'build.log'
  with log.open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
  entry={'backend':b,'command':cmd,'exit':code,'log_path':str(log.relative_to(A)),'log_sha256':sha(log),'sources_before':before,'source_archive_sha256':sha(dest/'test-compiled-sources.tar.gz')};R['builds'].append(entry);save();assert code==0,log;guard();assert {n:sha(ROOT/n)for n in names}==before
  match=re.search(r'Executable unittests src/lib.rs \((.+)\)',log.read_text());assert match;test=Path(match[1]);test=test if test.is_absolute()else ROOT/test;binary=dest/'tests';shutil.copyfile(test,binary);binary.chmod(test.stat().st_mode);entry.update(sources_after=before,test_binary_sha256=sha(binary));save();binaries[b+'-rust','compound_property_contract']=binary
 for x in old['builds']:
  if 'binary_sha256'in x:binaries[x['configuration'],x['fixture']]=Path(x['command'][-1])
 # Reuse environment and unchanged physical validator implementation from the closed runner as functions only; never execute its top-level driver.
 s=(OLD/'run.py').read_text();env_src=s[s.index('def environment(backend):'):s.index('markers=')];mesh_src=s[s.index('def mesh_check('):s.index('\ntry:')];namespace={'os':os,'subprocess':subprocess,'ROOT':ROOT,'math':math};exec(env_src+mesh_src,namespace);environment=namespace['environment'];mesh_check=namespace['mesh_check'];R.update(status='running');save();cpu_modes={};seen={}
 markers={'native_diagnostic_populations':'native diagnostic populations:','api_settings_test':'C API settings: pass','both_compound_ownership_test':'combined compound ownership:','compound_aabb_contract':'compound AABB contract:'}
 for index,case in enumerate(cases):
  guard();cfg=case['configuration'];name=case['fixture'];binary=binaries[cfg,name];dest=A/cfg/name;dest.mkdir(parents=True,exist_ok=True);trial=seen.get((cfg,name),0)+1;seen[cfg,name]=trial;out=dest/f'{trial}-stdout.log';err=dest/f'{trial}-stderr.log';env=environment('CPU'if cfg=='CPU'else cfg.split('-')[0]);cmd=[str(binary)]+case['arguments']
  with out.open('w')as o,err.open('w')as e:
   proc=subprocess.Popen(cmd,env=env,stdout=o,stderr=e,start_new_session=True);R['running']={'pid':proc.pid,'index':index,**case};save()
   try:code=proc.wait(timeout=P['watchdog_seconds'])
   except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGTERM);proc.wait(timeout=30);code=-999
  R.pop('running');row={**case,'trial':trial,'command':cmd,'exit':code,'binary_sha256':sha(binary),'stdout_path':str(out.relative_to(A)),'stderr_path':str(err.relative_to(A)),'stdout_sha256':sha(out),'stderr_sha256':sha(err),'environment':{k:v for k,v in env.items()if k.startswith(('GPU_','WGPU_','VK_'))}};R['runs'].append(row);save();assert code==0,(case,code);guard();text=out.read_text()
  if name=='compound_property_contract':assert '1 passed; 0 failed; 0 ignored;'in text;assert 'owned_table_and_invalid_handles_preserve_public_properties ... ok'in text;row['tests_passed']=1
  elif name=='compound_mesh_reference':
   if cfg=='CPU':cpu_modes[case['arguments'][0]]=out;row['mesh_validation']=mesh_check(out,out)
   else:row['mesh_validation']=mesh_check(cpu_modes[case['arguments'][0]],out)
  else:assert markers[name]in text
  if cfg!='CPU':assert 'NVIDIA GeForce RTX 4070 SUPER'in(err.read_text()+text)
  save();print(index+1,cfg,name,case['arguments'],'pass',flush=True)
 R['status']='pass';guard();save()
except BaseException as e:
 R.update(status='stopped',error=str(e),unlaunched=cases[len(R['runs']):],restoration={})
 for n,h in C['production_candidate_sha256'].items():
  if sha(ROOT/n)==h:(ROOT/n).write_bytes((A/'baseline'/n).read_bytes());R['restoration'][n]=sha(ROOT/n);assert R['restoration'][n]==P['baseline_production'][n]
  else:R['restoration'][n]='unexpected edits preserved'
 save();raise
finally:R.pop('driver_pid',None);R.pop('running',None);save()
print('Corrected handle contract2 +previously unlaunched physical/regression22 pass. No repeated properties/timing.',flush=True)
