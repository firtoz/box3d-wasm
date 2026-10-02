from pathlib import Path
import importlib.util,json,subprocess,shutil,math
R=Path.cwd();A=R/'artifacts/production-readiness/pr03-ragdoll-mesh-baseline';receipt=json.loads((A/'receipt.json').read_text());assert receipt['status']=='completed-baseline' and len(receipt['results'])==24 and all(r['exit']==0 for r in receipt['results'])
def load(name,file):
 spec=importlib.util.spec_from_file_location(name,R/file);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
health=load('health','scripts/check-ragdoll-health.py');mesh=load('mesh','scripts/compare-frozen-contacts.py');s={}
def rows(cell,scene,steps,bodies):
 rs=[list(map(float,l.split())) for l in (A/cell/scene/'stdout.txt').read_text().splitlines()];assert len(rs)==(steps+1)*bodies
 assert all(len(r)==16 and all(map(math.isfinite,r)) and r[:2]==list(divmod(i,bodies)) for i,r in enumerate(rs));return rs
for cell in ['ordinary','native']:
 d={'scope':'One baseline run per selected case;not five-repeat/full-state/final-build qualification','physical':health.evaluate(health.measure(A/'cpu/ragdolls/stdout.txt'),health.measure(A/cell/'ragdolls/stdout.txt')),'isolated':{},'mesh':{}}
 for scene in ['twist','twist-negative','tilted','tilted-negative']:
  a=rows('cpu',scene,120,1);b=rows(cell,scene,120,1);err=max(abs(x-y) for aa,bb in zip(a,b) for x,y in zip(aa[2:],bb[2:]));d['isolated'][scene]={'max_component_error':err,'limit':1e-5,'status':'pass' if err<=1e-5 else 'fail'}
 a=rows('cpu','ragdolls-no-contacts',60,112);b=rows(cell,'ragdolls-no-contacts',60,112);err=max(math.dist(aa[2:5],bb[2:5]) for aa,bb in zip(a,b));d['isolated']['ragdolls-no-contacts']={'max_position_error':err,'limit':5e-5,'status':'pass' if err<=5e-5 else 'fail'}
 for shape,count in [('grid',45),('torus',12)]:
  cpu=A/'cpu'/('mesh-'+shape)/'stdout.txt';gpu=A/cell/('mesh-'+shape)/'stdout.txt';result=mesh.compare(cpu,gpu,count,1e-5)
  if shape=='grid':
   for engine,data in [('cpu',cpu),('gpu',gpu)]:
    cases,manifolds=mesh.read(data,count);result[engine+'_all_grid_poses_contact']=all(r[1]>0 for r in cases.values());result[engine+'_upnormal_error']=max(max(abs(m[0]),abs(m[1]-1),abs(m[2])) for group in manifolds.values() for m in group)
    if not result[engine+'_all_grid_poses_contact'] or result[engine+'_upnormal_error']>1e-5:result['status']='fail'
  d['mesh'][shape]=result
 out=A/'legacy-evaluation'/cell;out.mkdir(parents=True,exist_ok=False)
 for eng,src in [('cpu','cpu'),('gpu',cell)]:
  for scene in ['twist','twist-negative','tilted','tilted-negative','ragdolls-no-contacts','ragdolls']:
   for orig,suffix in [('stdout.txt','txt'),('stderr.log','log')]:shutil.copyfile(A/src/scene/orig,out/(scene+'-'+eng+'.'+suffix))
 with (out/'stdout.log').open('w') as so,(out/'stderr.log').open('w') as se:run=subprocess.run(['python3',str(R/'scripts/validate-ragdoll-reference.py'),str(out)],stdout=so,stderr=se)
 d['original_full_reference_validator']={'exit':run.returncode,'status':'pass' if run.returncode==0 else 'fail','stderr':(out/'stderr.log').read_text(),'scope':'Original strict visual trajectory/contact/setup/physical screen retained unchanged; a physical-only pass never implies this screen passed.'};s[cell]=d
(A/'evaluation.json').write_text(json.dumps(s,indent=2)+'\n');print(json.dumps({k:{'physical':v['physical']['status'],'isolated':v['isolated'],'mesh':v['mesh'],'legacy':v['original_full_reference_validator']} for k,v in s.items()},indent=2))
