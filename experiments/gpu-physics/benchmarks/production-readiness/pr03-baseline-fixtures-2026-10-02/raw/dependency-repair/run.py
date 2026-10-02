from pathlib import Path
import json,hashlib,subprocess,time,os,signal
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-baseline-cpu-link-repair';P=A/'protocol-before-builds.json';p=json.loads(P.read_text());sha=lambda f:hashlib.sha256(Path(f).read_bytes()).hexdigest();assert not(A/'receipt.json').exists();s={'status':'building','protocol_sha256':sha(P),'driver_sha256':sha(__file__),'started_unix':time.time(),'commands':[],'fixtures':{}}
def save():
 t=A/'receipt.tmp';t.write_text(json.dumps(s,indent=2)+'\n');os.replace(t,A/'receipt.json')
def check():
 for n,h in p['inputs_before'].items():assert sha(n)==h,n
 for n,h in p['linked_inputs'].items():assert sha(n)==h,n
 for path,key in [('engine_build_receipt','engine_receipt_sha256'),('viewer_parent_receipt','viewer_parent_sha256')]:
  assert sha(p[path])==p[key];d=json.loads(Path(p[path]).read_text());assert d['inputs_before']==d['inputs_after']
  for n,h in d['inputs_after'].items():assert sha(R/n)==h,n
save()
def run(cmd,name):
 out=A/name;out.mkdir(parents=True,exist_ok=False)
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:
  proc=subprocess.Popen(cmd,cwd=R,stdout=so,stderr=se,start_new_session=True);s['running']={'pid':proc.pid,'command':cmd};save()
  try:code=proc.wait(timeout=p['watchdog_seconds'])
  except subprocess.TimeoutExpired:os.killpg(proc.pid,signal.SIGKILL);proc.wait();code='timeout'
 s.pop('running',None);row={'command':cmd,'exit':code,'stdout_sha256':sha(out/'stdout.log'),'stderr_sha256':sha(out/'stderr.log'),'output':str(out)};s['commands'].append(row);save();assert code==0,row
try:
 check();objects=[Path(n) for n in p['reused_shared_objects']]
 for n,h in p['reused_shared_objects'].items():assert sha(n)==h
 for key in ['previous_protocol','previous_receipt']:
  assert sha(p[key]['path'])==p[key]['sha256']
 cpu=p['cells']['cpu'];assert sha(cpu['receipt'])==cpu['receipt_sha256'];c=json.loads(Path(cpu['receipt']).read_text())
 for u in c['compiled_units']:
  assert sha(u['file'])==u['source_sha256'];assert sha(u['object'])==u['object_sha256']
 for n,h in c['inputs_after'].items():assert sha(R/n)==h
 for case in p['cases']:
  name=case['name'];cell=case['cell'];out=A/'links'/cell/name;binary=out/'fixture';source=R/case['source'];cmd=['g++','-O2','-std=c++17','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',str(source),'-I',str(R.parents[1]/'box3d/include'),'-I',str(R.parents[1]/'box3d/shared'),'-I',str(R/'c_abi')]
  if name=='drag':cmd+=[str(R.parents[1]/'box3d/samples/host/camera.cpp'),'-I',str(R/'native-samples'),'-I',str(R.parents[1]/'box3d/samples'),'-I',str(R.parents[1]/'box3d/extern/sokol')]
  if case['shared_objects']:cmd += [str(o) for o in objects]
  info=p['cells'][cell];build=Path(info['build']);both=cell.endswith('both');linked=[]
  if cell!='cpu':
   if both:cmd+=['-DGPU_API_DUAL','-Wl,--whole-archive',str(build/'libgpu_both_api.a'),'-Wl,--no-whole-archive'];linked.append(build/'libgpu_both_api.a')
   cmd+=['-Wl,--whole-archive',str(build/'libgpu_samples_api.a'),'-Wl,--no-whole-archive',info['library']];linked.extend([build/'libgpu_samples_api.a',Path(info['library'])])
   if both:cmd.append(str(build/'libbox3d_cpu.a'));linked.append(build/'libbox3d_cpu.a')
  cmd +=[str(build/'box3d_src/libbox3d.a'),'-ldl','-lpthread','-lm','-lgcc_s','-lGL','-o',str(binary)];linked.append(build/'box3d_src/libbox3d.a');run(cmd,'links/'+cell+'/'+name)
  s['fixtures'][cell+'/'+name]={'binary':str(binary),'binary_sha256':sha(binary),'source':str(source),'source_sha256':sha(source),'linked_inputs':{str(f):sha(f) for f in linked},'shared_objects':{str(f):sha(f) for f in objects} if case['shared_objects'] else {},'command_index':len(s['commands'])-1};save();check();print('Linked',cell,name,flush=True)
 assert len(s['commands'])==10 and len(s['fixtures'])==10;s.update(status='built',inputs_after={n:sha(n) for n in p['inputs_before']},finished_unix=time.time());save()
except BaseException as ex:s.update(status='stopped',error=repr(ex),finished_unix=time.time(),unbuilt=[c for c in p['cases'] if c['cell']+'/'+c['name'] not in s['fixtures']]);save();raise
print('PR03 current baseline fixtures built;no engine process launched',flush=True)
