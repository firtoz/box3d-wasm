from pathlib import Path
import hashlib,json,subprocess,time
R=Path.cwd();D=R/'experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15';out=D/'raw/ui02-before';out.mkdir(exist_ok=False);B=json.loads((D/'ui01-build-after.json').read_text());obj=out/'input-observer.o';r={'status':'RUNNING','operations':[],'viewers':[]};start=time.monotonic()
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
def save():(D/'ui02-build-before.json').write_text(json.dumps(r,indent=2)+'\n')
def run(label,cmd,cap):
 t=time.monotonic()
 with (out/(label+'.log')).open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=cap).returncode
 r['operations'].append({'label':label,'command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label
run('compile',['/usr/bin/c++','-std=c++20','-O2','-I'+str(R/'box3d/extern/sokol'),'-I'+str(R/'box3d/.fetchcontent-cache/imgui-src'),'-c',str(D/'input-observer.cpp'),'-o',str(obj)],10)
for label in ['cpu','ordinary-gpu']:
 op=next(x for x in B['operations']if x['label']==label+'-link');cmd=list(op['command']);binary=out/label;cmd[cmd.index('-o')+1]=str(binary);cmd[1:1]=[str(obj),'-Wl,--wrap=_Z11HandleEventPK10sapp_event'];run(label+'-link',cmd,15);r['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)})
r.update(status='PASS_READ_ONLY_INPUT_OBSERVER_BUILD',seconds=time.monotonic()-start,observer_source_sha256=sha(D/'input-observer.cpp'),observer_object_sha256=sha(obj));save();print(json.dumps({k:v for k,v in r.items()if k!='operations'}))
