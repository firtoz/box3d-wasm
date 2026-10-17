from pathlib import Path
import ast,hashlib,importlib.util,json,os,shlex,subprocess,sys,time
R=Path.cwd();B=R/'experiments/gpu-physics/benchmarks/production-readiness';D=B/'pr10-ui-2026-10-15';V=B/'pr09-repairs-2026-10-02/VIS07';out=D/'raw/ui01-after';out.mkdir(exist_ok=False);start=time.monotonic();before=json.loads((D/'ui01-build-before.json').read_text());mapping=json.loads((V/'candidate-compiled-source-map.json').read_text())['source_units'];sys.path.insert(0,str(R/'experiments/gpu-physics/scripts'));spec=importlib.util.spec_from_file_location('ui01inject',R/'experiments/gpu-physics/scripts/inject-sokol-bench.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
assert sha(D/'raw/ui-observer.o')==before['observer_object_sha256'];r={'status':'RUNNING','operations':[],'source_preservation':[],'viewers':[],'injector_sha256':sha(R/'experiments/gpu-physics/scripts/inject-sokol-bench.py')};objects={};pins=dict(before['input_hashes'])
def save():(D/'ui01-build-after.json').write_text(json.dumps(r,indent=2)+'\n')
def run(label,cmd,cap,cwd=None):
 t=time.monotonic()
 with (out/(label+'.log')).open('w')as f:code=subprocess.run(cmd,cwd=cwd,stdout=f,stderr=subprocess.STDOUT,timeout=cap,env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1')).returncode
 r['operations'].append({'label':label,'command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label
for mode,ref in [('cpu','candidate-reference-receipts/cpu.json'),('gpu','candidate-reference-receipts/ordinary-gpu.json'),('both','build-candidate-ordinary-both.json')]:
 folder=out/mode;folder.mkdir();frozen=next(x for x in mapping if x['receipt']==ref and x['source_original'].endswith('/sample_bench.cpp'));src=V/frozen['portable_source'];assert sha(src)==frozen['source_sha256'];text=src.read_text();candidate=mod.inject_info_text_wrap(text);restored=candidate.replace('\tImGui::PushTextWrapPos( 0.0f );\n\n','').replace('\n\tImGui::PopTextWrapPos();','');assert restored==text
 # Verify the production generator emits exactly the same repaired Info function.
 cmd=[sys.executable,str(R/'experiments/gpu-physics/scripts/inject-sokol-bench.py'),'--src',str(R/'box3d/samples/main.cpp'),'--dst',str(folder/'main.generated.cpp'),'--sample-src',str(R/'box3d/samples/sample.cpp'),'--sample-dst',str(folder/'canonical.cpp')]
 if mode!='cpu':cmd+=['--gpu-sidebar']
 run(mode+'-generation',cmd,5);canonical=(folder/'canonical.cpp').read_text();a='static void DrawInfoPanel( SampleContext* context )\n{';z='\n// Minimal in-world HUD';extract=lambda s:s[s.index(a):s.index(z,s.index(a))];assert extract(candidate)==extract(canonical),mode
 (folder/'sample_bench.cpp').write_text(candidate)
 # Preserve all compiled code outside this window using the exact archived source.
 receipt=json.loads((V/ref).read_text());unit=next(x for x in receipt['compiled_units']if x['file'].endswith('/sample_bench.cpp'));cmd=shlex.split(unit['command'])if isinstance(unit['command'],str)else list(unit['command']);obj=folder/'sample_bench.cpp.o';cmd[cmd.index('-o')+1]=str(obj);cmd[cmd.index('-c')+1]=str(folder/'sample_bench.cpp');run(mode+'-compile',cmd,10,unit['directory']);objects[mode]=obj
 r['source_preservation'].append({'mode':mode,'exact_frozen_compiled_source':str(src),'before_source_sha256':frozen['source_sha256'],'before_object_sha256':frozen['object_sha256'],'candidate_source_sha256':sha(folder/'sample_bench.cpp'),'candidate_object_sha256':sha(obj),'only_scoped_wrap_pair_added':restored==text,'current_production_generator_Info_function_exact':True})
 for unused in ['main.generated.cpp','canonical.cpp']:(folder/unused).unlink()
for op in before['operations']:
 if not op['label'].endswith('-link'):continue
 label=op['label'].removesuffix('-link');mode='cpu'if label=='cpu'else'both'if label.endswith('-both')else'gpu';cmd=list(op['command']);old=next(t for t in cmd if t.endswith('sample_bench.cpp.o'));sourcehash=next(x['before_object_sha256']for x in r['source_preservation']if x['mode']==mode);assert sha(old)==sourcehash,old;cmd[cmd.index(old)]=str(objects[mode]);binary=out/label;cmd[cmd.index('-o')+1]=str(binary);run(label+'-link',cmd,15);r['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)})
for f,h in pins.items():assert sha(f)==h,f
assert subprocess.check_output(['git','-C','box3d','status','--porcelain'])==b''
r.update(status='PASS_UI_ONLY_CANDIDATE_BUILD',input_hashes=pins,seconds=time.monotonic()-start,output_bytes=sum(f.stat().st_size for f in out.rglob('*')if f.is_file()),engine_source_and_libraries_unchanged=True);save();print(json.dumps({k:v for k,v in r.items()if k not in ['input_hashes','operations','source_preservation']}))
