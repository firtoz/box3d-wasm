#!/usr/bin/env python3
"""Validate matched small-scene trials and plot medians with full trial ranges."""
import argparse,hashlib,json,math,re,runpy,statistics
from pathlib import Path
from types import SimpleNamespace
BENCH=runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('input',type=Path);p.add_argument('output',type=Path);p.add_argument('--title',required=True);p.add_argument('--validate-only',action='store_true');p.add_argument('--renderers',action='store_true');a=p.parse_args()
 manifest=json.loads((a.input/'manifest.json').read_text());jobs=json.loads((a.input/'jobs.json').read_text())
 scopes={r:manifest['counts'] for r in manifest['renderers']} if a.renderers else manifest['scenes']
 expected={(s,n,t,v) for s,ns in scopes.items() for n in ns for t in range(1,manifest['trials']+1) for v in ['before','after','cpu']}
 seen=set();verified=[];phases=[];hashes={};cpu_hashes=set();environments=set();framebuffers={}
 for row in jobs:
  scope=row['renderer'] if a.renderers else row['scene']
  key=(scope,row['count'],row['trial'],row['variant']);assert key in expected and key not in seen,key;seen.add(key)
  assert row['status']=='ok' and row['returncode']==0,row['key']
  folder=a.input/row['key'];m=json.loads((folder/'manifest.json').read_text());args=SimpleNamespace(**m['arguments']);mode=row['mode'] if a.renderers else ('physics-cpu' if row['variant']=='cpu' else 'physics-gpu')
  assert args.scene==('falling-cubes' if a.renderers else row['scene']) and args.counts==[row['count']] and args.modes==[mode]
  assert (args.warmup,args.timed,args.workers)==(manifest['warmup'],manifest['timed'],manifest['workers'])
  configurations=manifest.get('configurations', {'before':{'color_prefix':'20','global_replay':'0'},'after':{'color_prefix':'auto','global_replay':'1'}})
  config=configurations['after' if row['variant']=='after' else 'before']
  assert args.gpu_solver=='global' and args.gpu_color_prefix==config['color_prefix']
  assert args.global_replay==config['global_replay']
  env=m['environment']
  assert env.get('GPU_PHYSICS_PROFILE_HOST','0')!='1' and env.get('GPU_PHYSICS_PROFILE_BROADPHASE','0')!='1'
  environments.add(json.dumps({k:v for k,v in env.items() if k not in ['GPU_PHYSICS_GLOBAL_REPLAY','GPU_PHYSICS_COLOR_PREFIX']},sort_keys=True))
  cpu_hashes.add(tuple(m['binaries'][k]['sha256'] for k in ['cpu','cpu_bridge','sokol_cpu']))
  if row['variant']!='cpu':
   binary_key='sokol_gpu' if a.renderers and scope=='sokol' else 'gpu'
   expected_hash=manifest['binaries'][row['variant']][scope]['sha256'] if a.renderers else manifest['variants'][row['variant']]['sha256']
   assert m['binaries'][binary_key]['sha256']==expected_hash
  raw=folder/f"{row['count']}-{mode}-1.json";data=json.loads(raw.read_text())
  values=BENCH['metrics'](data,mode,raw,args,row['count'])
  for k in ['mean_ms','p50_ms','p95_ms']:assert abs(values[k]-row['result'][0][k])<1e-8,(key,k)
  if mode=='physics-cpu':
   assert (data['sub_steps'],data['warmup_steps'],data['timed_steps'])==(4,manifest['warmup'],manifest['timed'])
  if mode=='physics-gpu':
   assert abs(data['dt']-1/60)<1e-7 and data['sub_steps']==4 and data['scene']==row['scene']
   assert data['raw_runs'][0].get('host_profile') is None
   phase={}
   for name in ['graph_ms','solve_ms','broadphase_ms','narrowphase_ms','prepare_ms','device_ms']:
    samples=data['raw_runs'][0][name]
    assert len(samples)==manifest['timed'] and all(math.isfinite(x) and x>=0 for x in samples)
    phase[name]=statistics.mean(samples)
   phases.append(dict(scene=scope,count=row['count'],trial=row['trial'],variant=row['variant'],**phase))
  if a.renderers:
   log=raw.with_suffix('.log').read_text()
   if scope=='sokol':
    renderer=re.search(r'sokol-renderer: (.+)',log);assert renderer,'Missing renderer identity'
    assert not re.search(r'llvmpipe|softpipe|software rasterizer',renderer.group(1),re.I),'Software renderer'
    if args.adapter=='nvidia':assert 'NVIDIA' in renderer.group(1)
   elif args.adapter=='nvidia':assert 'GPU adapter: NVIDIA' in log
   fb=tuple(values['framebuffer']);framebuffers.setdefault(scope,set()).add(fb)
   assert len(framebuffers[scope])==1,'Framebuffer mismatch'
   if scope=='sokol':
    frames=data['frames'];assert all(f['renderer_instances']==row['count']+1 for f in frames)
    if row['variant']!='cpu':assert all(f['gpu_draw_shapes']==row['count']+1 for f in frames)
    recorded=row['result'][0]['sokol_settings_after']
    assert BENCH['render_settings'](recorded)==BENCH['render_settings'](json.loads(manifest['sokol_settings'])) and recorded['drawDistance']==1000
  verified.append(dict(scene=scope,count=row['count'],trial=row['trial'],variant=row['variant'],**{k:values[k] for k in ['mean_ms','p50_ms','p95_ms']}))
  files=[raw,raw.with_suffix('.log'),folder/'manifest.json',folder/'trials.json']
  if a.renderers and scope=='direct':files.append(raw.with_suffix('.cadence.json'))
  for f in files:hashes[str(f.relative_to(a.input))]=digest(f)
 assert len(cpu_hashes)==1 and len(environments)==1,'Unmatched CPU binary or environment'
 assert seen==expected,f'Missing {len(expected-seen)} trials'
 groups=[]
 for scene,counts in scopes.items():
  for n in counts:
   for v in ['before','after','cpu']:
    rr=[r for r in verified if (r['scene'],r['count'],r['variant'])==(scene,n,v)]
    means=[r['mean_ms'] for r in rr]
    groups.append(dict(scene=scene,count=n,variant=v,mean_ms=statistics.median(means),min_ms=min(means),max_ms=max(means),p50_ms=statistics.median(r['p50_ms'] for r in rr),p95_ms=statistics.median(r['p95_ms'] for r in rr)))
 print(f'Validated {len(verified)} trials / {len(groups)} groups')
 if a.validate_only:return
 import matplotlib;matplotlib.use('Agg')
 import matplotlib.pyplot as plt
 a.output.parent.mkdir(parents=True,exist_ok=True)
 comparisons=[]
 for scope,counts in scopes.items():
  for n in counts:
   g={v:next(r for r in groups if (r['scene'],r['count'],r['variant'])==(scope,n,v)) for v in ['before','after','cpu']}
   paired=[]
   for trial in range(1,manifest['trials']+1):
    rr={v:next(r for r in verified if (r['scene'],r['count'],r['variant'],r['trial'])==(scope,n,v,trial)) for v in g}
    paired.append(rr['before']['mean_ms']/rr['after']['mean_ms'])
   comparisons.append(dict(scope=scope,count=n,speedup=g['before']['mean_ms']/g['after']['mean_ms'],paired_speedups=paired,
    cpu_over_gpu=g['cpu']['mean_ms']/g['after']['mean_ms'],gpu_trial_range_below_cpu=g['after']['max_ms']<g['cpu']['min_ms']))
 a.output.with_suffix('.json').write_text(json.dumps(dict(manifest=manifest,groups=groups,comparisons=comparisons,framebuffers={k:list(next(iter(v))) for k,v in framebuffers.items()},phase_trials=phases,raw_sha256=hashes),indent=2)+'\n')
 colors={'before':'#8a8995','after':'#097f98','cpu':'#bc601d'}
 for metric in ['latency','throughput','percentiles']:
  fig,axs=plt.subplots(1,2,figsize=(13,5))
  for ax,(scene,counts) in zip(axs,scopes.items()):
   for v,label in [('before','GPU baseline'),('after','GPU candidate'),('cpu','CPU')]:
    rows=[g for g in groups if g['scene']==scene and g['variant']==v]
    if metric=='percentiles':
     for key,style in [('p50_ms','--'),('p95_ms','-')]:ax.plot(counts,[g[key] for g in rows],style,marker='o',markersize=3,label=label+' '+key[:3],color=colors[v])
     continue
    ys=[g['mean_ms'] if metric=='latency'  else 1000/g['mean_ms'] for g in rows]
    lo=[g['min_ms'] if metric=='latency' else 1000/g['max_ms'] for g in rows];hi=[g['max_ms'] if metric=='latency' else 1000/g['min_ms'] for g in rows]
    ax.plot(counts,ys,'o-',label=label,color=colors[v],markersize=4);ax.fill_between(counts,lo,hi,color=colors[v],alpha=.15)
    if metric=='latency' and v!='cpu':ax.plot(counts,[g['p95_ms'] for g in rows],':',color=colors[v],alpha=.8)
   ax.set(xscale='log',yscale='log',xlabel='Dynamic bodies',ylabel=('Frame time (ms)' if a.renderers else 'Completed step (ms)') if metric in ['latency','percentiles'] else ('Frames / second' if a.renderers else 'Completed steps / second'),title=(scene.title()+' renderer '+str(next(iter(framebuffers[scene])))) if a.renderers else ('Falling cubes' if scene=='falling-cubes' else 'Independent two-box groups'));ax.grid(alpha=.2);ax.legend()
  fig.suptitle(a.title+'\n'+('Median of trial p50 / p95 latencies' if metric=='percentiles' else 'Median trial mean; shading = full trial range'+('; dotted = median trial p95' if metric=='latency' else '')))
  fig.tight_layout()
  for ext in ['png','svg']:
   path=a.output.parent/(a.output.name+'-'+metric+'.'+ext);fig.savefig(path,dpi=180,bbox_inches='tight',pad_inches=.12)
   if ext=='svg':path.write_text('\n'.join(l.rstrip() for l in path.read_text().splitlines())+'\n')
  plt.close(fig)
 if not a.renderers:
  fig,axs=plt.subplots(1,2,figsize=(13,5))
  for ax,(scene,counts) in zip(axs,scopes.items()):
   focus=[n for n in counts if n<=5000]
   for v,label in [('before','Baseline'),('after','Candidate')]:
    for phase,style in [('graph_ms','--'),('solve_ms','-')]:
     values=[[r[phase] for r in phases if (r['scene'],r['count'],r['variant'])==(scene,n,v)] for n in focus]
     ax.plot(focus,[statistics.median(x) for x in values],style,marker='o',color=colors[v],label=label+' '+phase.removesuffix('_ms'))
     ax.fill_between(focus,[min(x) for x in values],[max(x) for x in values],color=colors[v],alpha=.1)
   ax.set(xscale='log',xlabel='Dynamic bodies',ylabel='GPU milliseconds / completed step',title='Falling cubes' if scene=='falling-cubes' else 'Independent two-box groups')
   ax.grid(alpha=.2);ax.legend()
  fig.suptitle(a.title+'\nGPU graph and solver timestamps; median trial mean and full trial range')
  fig.tight_layout()
  for ext in ['png','svg']:
   path=a.output.parent/(a.output.name+'-phases.'+ext);fig.savefig(path,dpi=180,bbox_inches='tight',pad_inches=.12)
   if ext=='svg':path.write_text('\n'.join(l.rstrip() for l in path.read_text().splitlines())+'\n')
  plt.close(fig)
if __name__=='__main__':main()
