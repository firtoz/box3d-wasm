from pathlib import Path
import hashlib,json,subprocess,time,re
R=Path.cwd();B=R/'experiments/gpu-physics/benchmarks/production-readiness';D=B/'pr10-ui-2026-10-15';N=B/'pr09-repairs-2026-10-02/PERF04/general-mesh-normal-viewers-2026-10-10';out=D/'raw/ui01-before';out.mkdir(exist_ok=False);start=time.monotonic()
def sha(f):
 with Path(f).open('rb')as x:return hashlib.file_digest(x,'sha256').hexdigest()
source=D/'ui-observer.cpp';symbols=re.findall(r'"(__wrap__Z[^" ]+)"',source.read_text());symbols+=['__wrap_'+s for s in re.findall(r'DECL\(\w+,"(_Z[^"]+)"',source.read_text())];symbols=list(dict.fromkeys(s.removeprefix('__wrap_')for s in symbols));assert len(symbols)==12,symbols
p=json.loads((N/'protocol.json').read_text());cp=json.loads((N.parent/'nonstepping-window-observer/protocol.json').read_text());ops=[next(x for x in cp['operations']if x['label']=='cpu-link')]+[x for x in p['operations']if x['kind']=='link'];assert len(ops)==5
imgui=R/'box3d/.fetchcontent-cache/imgui-src';assert '#define IMGUI_VERSION       "1.92.7"'in(imgui/'imgui.h').read_text();obj=D/'raw/ui-observer.o';r={'status':'RUNNING','source_sha256':sha(source),'imgui_header_sha256':sha(imgui/'imgui.h'),'imgui_internal_sha256':sha(imgui/'imgui_internal.h'),'wrapped_symbols':symbols,'operations':[],'viewers':[]};pins={}
def save():(D/'ui01-build-before.json').write_text(json.dumps(r,indent=2)+'\n')
def run(label,cmd,seconds):
 t=time.monotonic()
 with (out/(label+'.log')).open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=seconds).returncode
 r['operations'].append({'label':label,'command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label
run('observer-compile',['/usr/bin/c++','-std=c++20','-O2','-I'+str(R/'box3d/include'),'-I'+str(imgui),'-c',str(source),'-o',str(obj)],10)
for op in ops:
 label=op['label'].removeprefix('after-').removesuffix('-link');cmd=list(op['command']);binary=out/label;cmd[cmd.index('-o')+1]=str(binary)
 for i,t in enumerate(cmd):
  if t==str(N/'ordinary-candidate.a'):cmd[i]=str(R/'experiments/gpu-physics/target/samples/release/libgpu_physics.a')
  if t==str(N/'native-candidate.a'):cmd[i]=str(R/'experiments/gpu-physics/target/native-cache-build/release/libgpu_physics.a')
 for t in cmd:
  if t.startswith('/')and Path(t).is_file()and t!=str(binary):pins[t]=sha(t)
 cmd[1:1]=[str(obj)]+['-Wl,--wrap='+s for s in symbols];run(label+'-link',cmd,15);r['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)});save()
for f,h in pins.items():assert sha(f)==h,f
r.update(status='PASS_BEFORE_DIAGNOSTIC_BUILD',input_hashes=pins,seconds=time.monotonic()-start,outputs_bytes=sum(f.stat().st_size for f in out.iterdir()if f.is_file()),observer_object_sha256=sha(obj));save();print(json.dumps({k:v for k,v in r.items()if k not in ['input_hashes','operations','wrapped_symbols']}))
