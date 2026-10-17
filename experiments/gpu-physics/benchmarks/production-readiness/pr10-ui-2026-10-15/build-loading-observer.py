from pathlib import Path
import hashlib,json,subprocess,time
R=Path.cwd();B=R/'experiments/gpu-physics/benchmarks/production-readiness';D=B/'pr10-ui-2026-10-15';N=B/'pr09-repairs-2026-10-02/PERF04/general-mesh-normal-viewers-2026-10-10';out=D/'raw/loading-viewers';out.mkdir(exist_ok=False);start=time.monotonic()
def sha(f):
 with Path(f).open('rb')as p:return hashlib.file_digest(p,'sha256').hexdigest()
b=json.loads((D/'loading-observer-budget.json').read_text());p=json.loads((N/'protocol.json').read_text());o=D/'raw/loading-observer.o';r={'status':'RUNNING','source_sha256':sha(D/'loading-observer.cpp'),'operations':[],'viewers':[]};pins={}
def save():(D/'loading-build-receipt.json').write_text(json.dumps(r,indent=2)+'\n')
def run(label,cmd,cap):
 t=time.monotonic()
 with (out/(label+'.log')).open('w')as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,timeout=cap).returncode
 r['operations'].append({'label':label,'command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label
run('compile',['/usr/bin/c++','-O2','-I'+str(R/'box3d/include'),'-c',str(D/'loading-observer.cpp'),'-o',str(o)],10)
for op in p['operations']:
 if op['kind']!='link':continue
 label=op['label'].removeprefix('after-').removesuffix('-link');cmd=list(op['command']);binary=out/label;i=cmd.index('-o');cmd[i+1]=str(binary)
 for j,token in enumerate(cmd):
  if token==str(N/'ordinary-candidate.a'):cmd[j]=str(R/'experiments/gpu-physics/target/samples/release/libgpu_physics.a')
  if token==str(N/'native-candidate.a'):cmd[j]=str(R/'experiments/gpu-physics/target/native-cache-build/release/libgpu_physics.a')
 for token in cmd:
  if token.startswith('/')and Path(token).is_file()and token!=str(binary):pins[token]=sha(token)
 cmd[1:1]=['-Wl,--wrap=_Z16gpu_loading_poll9b3WorldId',str(o)]
 run(label+'-link',cmd,15);r['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)});save()
assert len(r['viewers'])==4
for f,h in pins.items():assert sha(f)==h,f
r.update(status='PASS_DIAGNOSTIC_ONLY_RELINKS',input_hashes=pins,seconds=time.monotonic()-start,output_bytes=sum(f.stat().st_size for f in out.iterdir()if f.is_file()));save();size=o.stat().st_size;o.unlink();r['consumed_marker_object_bytes_deleted']=size;save();print(json.dumps({k:v for k,v in r.items()if k not in ['input_hashes','operations']}))
