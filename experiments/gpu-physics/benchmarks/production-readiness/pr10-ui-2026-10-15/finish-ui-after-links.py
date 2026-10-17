from pathlib import Path
import hashlib,json,subprocess,time
R=Path.cwd();D=R/'experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15';out=D/'raw/ui01-after';bins=out/'bin';bins.mkdir(exist_ok=False);r=json.loads((D/'ui01-build-after-failed-link.json').read_text());before=json.loads((D/'ui01-build-before.json').read_text());start=time.monotonic()
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def save():(D/'ui01-build-after.json').write_text(json.dumps(r,indent=2)+'\n')
for x in r['source_preservation']:
 assert sha(out/x['mode']/'sample_bench.cpp')==x['candidate_source_sha256'];assert sha(out/x['mode']/'sample_bench.cpp.o')==x['candidate_object_sha256']
for f,h in before['input_hashes'].items():assert sha(f)==h,f
r['operations_before_path_correction']=r.pop('operations');r['operations']=[];r['viewers']=[];r['status']='RELINKING_DISTINCT_OUTPUT_PATH';save()
for op in before['operations']:
 if not op['label'].endswith('-link'):continue
 label=op['label'].removesuffix('-link');mode='cpu'if label=='cpu'else'both'if label.endswith('-both')else'gpu';cmd=list(op['command']);old=next(t for t in cmd if t.endswith('sample_bench.cpp.o'));assert sha(old)==next(x['before_object_sha256']for x in r['source_preservation']if x['mode']==mode);cmd[cmd.index(old)]=str(out/mode/'sample_bench.cpp.o');binary=bins/label;cmd[cmd.index('-o')+1]=str(binary);t=time.monotonic()
 with (out/(label+'-corrected-link.log')).open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=15).returncode
 r['operations'].append({'label':label+'-link','command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label;r['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)});save()
for f,h in before['input_hashes'].items():assert sha(f)==h,f
r.update(status='PASS_UI_ONLY_CANDIDATE_BUILD',input_hashes=before['input_hashes'],seconds=time.monotonic()-start,output_bytes=sum(f.stat().st_size for f in out.rglob('*')if f.is_file()),engine_source_and_libraries_unchanged=True);save();print(json.dumps({k:v for k,v in r.items()if k not in ['input_hashes','operations','operations_before_path_correction','source_preservation']}))
