#!/usr/bin/env python3
"""Plot diagnostic host/GPU timings separately; GPU work overlaps host wall time."""
import argparse,hashlib,json,math,statistics
from pathlib import Path
COUNTS=[5,100,1000,2000,5000,10000]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('input',type=Path);p.add_argument('output',type=Path);p.add_argument('--validate-only',action='store_true');a=p.parse_args()
 result=[];hashes={}
 for scene in ['falling-cubes','mixed-stacks']:
  for count in COUNTS:
   files=[a.input/'host-profile'/f'{count}.json'] if scene=='falling-cubes' else [a.input/'independent-groups-pilot'/f'{count}-{t}-baseline.json' for t in [1,2]]
   trials=[]
   for f in files:
    d=json.loads(f.read_text());r=d['raw_runs'][0];assert d['scene']==scene and d['bodies']==count+(1 if scene=='falling-cubes' else 2)
    assert d['sub_steps']==4 and not d['sleep'] and r['physics_step']==330 and r['live_contacts']>0
    keys=['completed_step_ms','encode_ms','device_ms','broadphase_ms','narrowphase_ms','graph_ms','prepare_ms','solve_ms']
    values={k:r[k] for k in keys}|r['host_profile']
    for k,v in values.items():assert len(v)==240 and all(math.isfinite(x) and x>=0 for x in v),(f,k)
    assert all(abs(x+y-z)<=.000151 for x,y,z in zip(values['step_call_ms'],values['completion_wait_ms'],values['completed_step_ms']))
    trials.append({k:statistics.mean(v) for k,v in values.items()}|{'solver_dispatches':r['solver_dispatches'],'encode_commands':r['encode_commands'],'step_plus_host_mirror_p50_ms':d['step_plus_host_mirror']['p50_ms']})
    for x in [f,f.with_suffix('.log')]:hashes[str(x.relative_to(a.input))]=hashlib.sha256(x.read_bytes()).hexdigest()
   result.append(dict(scene=scene,count=count,trials=len(trials),**{k:statistics.median(t[k] for t in trials) for k in trials[0]}))
 print('Validated 18 diagnostic runs covering both workloads at all six counts')
 if a.validate_only:return
 import matplotlib;matplotlib.use('Agg')
 import matplotlib.pyplot as plt
 a.output.parent.mkdir(parents=True,exist_ok=True)
 a.output.with_suffix('.json').write_text(json.dumps(dict(groups=result,raw_sha256=hashes,notes='Diagnostic only. Host step-call and wait overlap GPU execution. Separate mirror pass is not additive.'),indent=2)+'\n')
 fig,axs=plt.subplots(2,2,figsize=(13,9),layout='constrained')
 for i,scene in enumerate(['falling-cubes','mixed-stacks']):
  rows=[r for r in result if r['scene']==scene]
  for key,label in [('completed_step_ms','Completed step'),('step_call_ms','Host step call'),('completion_wait_ms','Completion wait + harvesting'),('encode_ms','Command encoding')]:axs[i,0].plot(COUNTS,[r[key] for r in rows],'o-',label=label,markersize=4)
  for key,label in [('device_ms','Whole GPU interval'),('broadphase_ms','Broadphase'),('narrowphase_ms','Narrowphase'),('graph_ms','Contact graph'),('prepare_ms','Preparation'),('solve_ms','Solver')]:axs[i,1].plot(COUNTS,[r[key] for r in rows],'o-',label=label,markersize=4)
  title='Falling cubes (one diagnostic trial)' if scene=='falling-cubes' else 'Independent groups (median of two trials)'
  for j in [0,1]:axs[i,j].set(xscale='log',yscale='log',xlabel='Dynamic bodies',ylabel='Milliseconds',title=title+(' — host wall time' if j==0 else ' — GPU timestamps'));axs[i,j].grid(alpha=.2);axs[i,j].legend(fontsize=8)
 fig.suptitle('RTX 4070 Laptop • baseline scheduling diagnostics\nGPU execution overlaps host wall time; these series must not be added together')
 for ext in ['png','svg']:
  f=a.output.with_suffix('.'+ext);fig.savefig(f,dpi=180)
  if ext=='svg':f.write_text('\n'.join(s.rstrip() for s in f.read_text().splitlines())+'\n')
 plt.close(fig)
if __name__=='__main__':main()
