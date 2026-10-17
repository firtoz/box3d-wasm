from pathlib import Path
import json,hashlib,time,subprocess
R=Path.cwd();D=R/'experiments/gpu-physics/benchmarks/production-readiness/pr10-ui-2026-10-15';O=D/'raw/solver-controls-build';O.mkdir(exist_ok=False);B=json.loads((D/'ui02-build-after.json').read_text());r={'status':'RUNNING','operations':[],'viewers':[]};start=time.monotonic()
def sha(p):
 with Path(p).open('rb')as f:return hashlib.file_digest(f,'sha256').hexdigest()
inputs={p:sha(p)for op in B['operations']if op['label'].endswith('-link')for p in op['command']if p.endswith(('.a','.o'))and Path(p).exists()}
def save():(D/'solver-controls-build.json').write_text(json.dumps(r,indent=2)+'\n')
def run(label,cmd,cap):
 t=time.monotonic()
 with (O/(label+'.log')).open('w')as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT,timeout=cap).returncode
 r['operations'].append({'label':label,'command':cmd,'exit':code,'seconds':time.monotonic()-t});save();assert code==0,label
obj=O/'observer.o';run('compile',['/usr/bin/c++','-std=c++20','-O2','-I'+str(R/'box3d/include'),'-c',str(D/'solver-controls-observer.cpp'),'-o',str(obj)],15)
wraps=['b3World_Step','cpu_b3World_Step','gpu_b3_world_step','b3World_SetContactRecycleDistance','cpu_b3World_SetContactRecycleDistance','gpu_b3_world_set_contact_recycle_distance']
for label in ['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both']:
 cmd=list(next(x for x in B['operations']if x['label']==label+'-link')['command']);cmd=[x for x in cmd if not x.endswith('ui02-before/input-observer.o')and x!='-Wl,--wrap=_Z11HandleEventPK10sapp_event'];cmd[1:1]=[str(obj)]+['-Wl,--wrap='+w for w in wraps];binary=O/label;cmd[cmd.index('-o')+1]=str(binary);run(label+'-link',cmd,20);r['viewers'].append({'label':label,'binary':str(binary),'sha256':sha(binary)});save()
assert all(sha(p)==h for p,h in inputs.items());r.update(status='PASS_READ_ONLY_SOLVER_CONTROL_OBSERVER',input_pins=inputs,seconds=time.monotonic()-start);save();print(r['status'],r['seconds'])
