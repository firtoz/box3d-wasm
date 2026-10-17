from pathlib import Path
import hashlib,importlib.util,json,os,shlex,subprocess,sys,time
sys.dont_write_bytecode=True
R=Path.cwd();G=R/'experiments/gpu-physics';D=G/'benchmarks/production-readiness/pr10-ui-2026-10-15';P=D.parent/'pr09-repairs-2026-10-02';out=D/'raw/ui02-after';out.mkdir(exist_ok=False);(out/'bin').mkdir();B=json.loads((D/'ui01-build-after.json').read_text());before=json.loads((D/'ui02-build-before.json').read_text());obj=D/'raw/ui02-before/input-observer.o'
sys.path.insert(0,str(G/'scripts'));spec=importlib.util.spec_from_file_location('injector',G/'scripts/inject-sokol-bench.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
inputs={x:sha(x)for op in B['operations']if op['label'].endswith('-link')for x in op['command']if x.endswith(('.o','.a'))and Path(x).is_file()}
r={'status':'RUNNING','operations':[],'viewers':[],'source_preservation':[],'input_pins':inputs,'helper_sha256':sha(G/'native-samples/sokol_ui_input.h')};start=time.monotonic()
def save():(D/'ui02-build-after.json').write_text(json.dumps(r,indent=2)+'\n');(D/'ui02-build-before.json').write_text(json.dumps(before,indent=2)+'\n')
def run(label,cmd,cap,cwd=R):
 t=time.monotonic()
 with (out/(label+'.log')).open('w')as f:code=subprocess.run(cmd,cwd=cwd,stdout=f,stderr=subprocess.STDOUT,timeout=cap).returncode
 r['operations'].append({'label':label,'command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label
for label in ['native-gpu','ordinary-both','native-both']:
 op=next(x for x in B['operations']if x['label']==label+'-link');cmd=list(op['command']);binary=D/'raw/ui02-before'/label;assert not binary.exists();cmd[cmd.index('-o')+1]=str(binary);cmd[1:1]=[str(obj),'-Wl,--wrap=_Z11HandleEventPK10sapp_event'];run('before-'+label+'-link',cmd,20);before['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)});save()
for label in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
 cmd=list(next(x for x in B['operations']if x['label']==label+'-link')['command']);oldobj=next(x for x in cmd if x.endswith('main_bench.cpp.o'))
 if label=='cpu':
  v=P/'VIS08';maps=json.loads((v/'candidate-compiled-source-map.json').read_text())['source_units'];u=next(x for x in maps if x['object_sha256']==sha(oldobj));a=json.loads((v/u['receipt']).read_text());unit=next(x for x in a['compiled_units']if x['object_sha256']==sha(oldobj));source=v/u['portable_source']
 else:
  v=P/'VIS02';a=json.loads((v/('diagnostic-build-'+label+'.json')).read_text());unit=next(x for x in a['compiled_units']if x['object_sha256']==sha(oldobj));source=Path(unit['file'])
 assert sha(source)==unit['source_sha256'];original=source.read_text();modified=m.inject_ui_event_capture(original)
 assert modified.replace('#include "sokol_ui_input.h"\n','').replace('uiCaptured = gpu_ui_capture_event( e, HandleEvent( e ) );','uiCaptured = HandleEvent( e );')==original
 folder=out/label;folder.mkdir();src=folder/'main_bench.cpp';src.write_text(modified);newobj=folder/'main_bench.cpp.o';compile_cmd=unit['command'];compile_cmd=shlex.split(compile_cmd)if isinstance(compile_cmd,str)else list(compile_cmd);compile_cmd[compile_cmd.index('-o')+1]=str(newobj);compile_cmd[compile_cmd.index('-c')+1]=str(src);run(label+'-compile',compile_cmd,25,unit['directory']);r['source_preservation'].append({'label':label,'original_source':str(source),'original_source_sha256':sha(source),'original_object_sha256':sha(oldobj),'candidate_source_sha256':sha(src),'outside_include_call_byte_identical':True})
 cmd[cmd.index(oldobj)]=str(newobj);cmd[cmd.index('-o')+1]=str(out/'bin'/label);cmd[1:1]=[str(obj),'-Wl,--wrap=_Z11HandleEventPK10sapp_event'];run(label+'-link',cmd,20);r['viewers'].append({'label':label,'binary':str(out/'bin'/label),'sha256':sha(out/'bin'/label)});save()
assert all(sha(p)==h for p,h in inputs.items());r.update(status='PASS_UI02_ONLY_CANDIDATE_BUILD',all_original_inputs_preserved=True,seconds=time.monotonic()-start);save();print(r['status'],r['seconds'],flush=True)
