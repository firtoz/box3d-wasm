#!/usr/bin/env python3
"""Paired completed-step measurements using frozen warm-start fixture executables.

Link the identical fixture source against archived baseline and candidate ordinary
release libraries, using the same C metadata/oracle archives and linker options,
as artifacts/warm-start/{baseline,candidate}/fixture first.
No concurrent GPU or build jobs should run during measurements.
"""
import subprocess,os,json,hashlib,statistics,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'artifacts/warm-start/timings';out.mkdir(parents=True,exist_ok=True)
binaries={'baseline':root/'artifacts/warm-start/baseline/fixture','candidate':root/'artifacts/warm-start/candidate/fixture'}
env=os.environ.copy();env['GPU_PHYSICS_PIPELINE_CACHE_DIR']=os.path.expanduser('~/.cache/box3d-gpu-physics/pipelines')
runs=[]
for repeat in range(6):
 for name in (['baseline','candidate'] if repeat%2==0 else ['candidate','baseline']):
  label=f'{repeat}-{name}';print(label,flush=True)
  with (out/(label+'.jsonl')).open('w') as stdout,(out/(label+'.log')).open('w') as stderr:
   subprocess.run([str(binaries[name]),'--timing'],stdout=stdout,stderr=stderr,env=env,cwd=root,check=True)
  rows=[json.loads(x) for x in (out/(label+'.jsonl')).read_text().splitlines()]
  assert len(rows)==4 and all(r['timing_only'] and r['steps']==180 and r['mean_ms']>0 for r in rows)
  runs.append({'repeat':repeat,'arm':name,'prewarm':repeat==0,'rows':rows})
receipt={'schema':'gpu-warm-start-timings-v1','baseline_commit':'6546eb8','device':'NVIDIA GeForce RTX 4070 SUPER Vulkan','protocol':{'warmup_steps':60,'timed_steps':180,'paired_processes':5,'separate_prewarm_processes':2,'warm_starting':'default enabled','measurement':'completed b3World_Step plus public position read; force application excluded','order':'alternating baseline/candidate','linkage':'both ordinary release core C shim, identical metadata/oracle archives and link options'},'binaries':{k:{'path':str(v.relative_to(root)),'sha256':hashlib.sha256(v.read_bytes()).hexdigest()} for k,v in binaries.items()},'runs':runs}
(root/'benchmarks/2026-09-29-warm-start-timings.json').write_text(json.dumps(receipt,indent=2)+'\n')
