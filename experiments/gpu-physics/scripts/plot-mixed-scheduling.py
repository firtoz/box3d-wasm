#!/usr/bin/env python3
"""Publish/revalidate all fixed-budget evidence and plot desktop scheduling."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import re
import statistics

def sha(data): return hashlib.sha256(data).hexdigest()

def validate(bundle):
    rows=[]
    files=bundle['files']
    protocol=json.loads(files['protocol.json'])
    assert protocol['baseline_runs']==27 and protocol['diagnostic_limit']==6
    assert protocol['candidate_limit']==2 and protocol['pilots_per_candidate']==2
    assert protocol['confirmation_runs']==18
    assert protocol['fixtures']==[['falling-cubes',50000],['mixed-stacks',4096],['mixed-topology',12288]]
    for name,text in files.items():
        if name.endswith('-build.json'):
            build=json.loads(text)
            if 'source_patch' in build:
                assert sha(files[build['source_patch']].encode())==build['patch_sha256'], name
                assert files[build['build_log']].strip(), name
                assert build['engine_sources'] and build['revision'], name
    for name,text in files.items():
        if not name.endswith('.receipt.json'): continue
        row=json.loads(text)
        if 'phase' not in row: continue
        assert row['status']=='ok', name
        raw_name=name.replace('.receipt.json','.json')
        assert sha(files[raw_name].encode())==row['raw_sha256']
        assert sha(files[name.replace('.receipt.json','.log')].encode())==row['log_sha256']
        build=json.loads(files[row['binary']+'-build.json'])
        assert build['binary_sha256']==row['binary_sha256']
        data=json.loads(files[raw_name]);run=data['raw_runs'][0]
        samples=run['completed_step_ms']
        assert len(samples)==240 and data['warmup_steps']==90 and data['sub_steps']==4
        assert not data['sleep'] and abs(data['dt']-1/60)<1e-7 and run['physics_step']==330
        assert data['adapter']=='NVIDIA GeForce RTX 4070 SUPER'
        assert '610.57.04' in row['adapter_driver']
        assert data['bodies']==row['count']+(1 if row['scene']=='falling-cubes' else 2)
        env=row['environment']
        assert env['GPU_PHYSICS_ADAPTER']=='nvidia'
        assert env['GPU_PHYSICS_COLOR_PREFIX']=='20'
        assert env['GPU_PHYSICS_BENCH_COMPLETED_ONLY']=='1'
        assert env['GPU_PHYSICS_COMPONENT_TGS']=={'global':'0','component':'1','auto':'auto','candidate':'split'}[row['schedule']]
        assert ('GPU_PHYSICS_TOPOLOGY_SAMPLES' in env)==row['phase'].startswith('diagnostic')
        assert len(run['solver_dispatches_samples'])==240
        assert abs(statistics.mean(samples)-row['mean_ms'])<1e-10
        assert data['completed_step']['p95_ms']==row['p95_ms']
        rows.append(row | {'raw':raw_name, 'dispatch_counts':sorted(set(run['solver_dispatches_samples']))})
    baseline=[r for r in rows if r['phase']=='baseline']
    assert len(baseline)==27
    for scene in ['falling-cubes','mixed-stacks','mixed-topology']:
        for schedule in ['global','component','auto']:
            assert sum(r['scene']==scene and r['schedule']==schedule for r in baseline)==3
    assert sum(r['phase'].startswith('diagnostic') for r in rows)==6
    assert sum(r['phase'].startswith('pilot-') for r in rows)==4
    for phase in {r['phase'] for r in rows if r['phase'].startswith('pilot-')}:
        assert sum(r['phase']==phase for r in rows)==2
    confirmation=[r for r in rows if r['phase']=='confirmation']
    assert not confirmation or len(confirmation)==18
    if confirmation:
        for scene in ['falling-cubes','mixed-stacks','mixed-topology']:
            for schedule in ['auto','candidate']:
                assert sum(r['scene']==scene and r['schedule']==schedule for r in confirmation)==3
    assert len(rows)==55 and len({r['pid'] for r in rows})==55
    ordered=sorted(rows,key=lambda r:r['started'])
    assert all(a['finished']<=b['started'] for a,b in zip(ordered,ordered[1:])), 'overlapping timing processes'
    for phase,forward in [('baseline',['global','component','auto']),('confirmation',['auto','candidate'])]:
        for scene in ['falling-cubes','mixed-stacks','mixed-topology']:
            for trial in [1,2,3]:
                actual=[r['schedule'] for r in ordered if r['phase']==phase and r['scene']==scene and r['trial']==trial]
                assert actual==(forward if trial%2 else list(reversed(forward)))
    headline=[r['environment'] for r in rows if not r['phase'].startswith('diagnostic')]
    # All solver settings match; only the deliberate scheduling override varies.
    settings=[{k:v for k,v in env.items() if k!='GPU_PHYSICS_COMPONENT_TGS'} for env in headline]
    assert all(env==settings[0] for env in settings)
    return rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('dataset', type=Path)
    p.add_argument('--raw',type=Path)
    p.add_argument('--validate-only',action='store_true')
    a=p.parse_args();a.dataset.mkdir(parents=True,exist_ok=True)
    if a.raw:
        files={str(f.relative_to(a.raw)):f.read_text() for f in sorted(a.raw.rglob('*'))
            if f.is_file() and 'binaries' not in f.relative_to(a.raw).parts
            and f.suffix in {'.json','.log','.patch','.py','.rs','.wgsl','.baseline','.txt'}}
        bundle={'schema':1,'files':files}
        packed=gzip.compress(json.dumps(bundle,sort_keys=True).encode(),mtime=0)
        (a.dataset/'raw.json.gz').write_bytes(packed)
    packed=(a.dataset/'raw.json.gz').read_bytes();bundle=json.loads(gzip.decompress(packed))
    rows=validate(bundle)
    phases={}
    for name,text in bundle['files'].items():
        if name.startswith('diagnostic') and name.endswith('.log'):
            samples=[json.loads(x) for x in re.findall(r'(?:component|split)-profile step=\d+ ms=(\[[^\n]+\])',text)]
            topology=[line for line in text.splitlines() if 'scheduling-topology' in line]
            if samples or topology:
                phases[name]={'gpu_phase_samples_ms':samples,
                    'gpu_phase_mean_ms':[statistics.mean(x) for x in zip(*samples)] if samples else [],
                    'topology':topology}
    summaries={}
    for scene in ['falling-cubes','mixed-stacks','mixed-topology']:
        summaries[scene]={}
        for phase,schedule in sorted({(r['phase'],r['schedule']) for r in rows if r['scene']==scene}):
            selected=[r for r in rows if r['scene']==scene and r['phase']==phase and r['schedule']==schedule]
            mean=statistics.median(r['mean_ms'] for r in selected)
            summaries[scene][phase+'/'+schedule]={'trials':len(selected),'median_mean_ms':mean,
                'steps_per_second':1000/mean,'median_p95_ms':statistics.median(r['p95_ms'] for r in selected)}
    results={'schema':1,'raw_sha256':sha(packed),'rows':rows,'summaries':summaries,'diagnostics':phases}
    if a.raw: (a.dataset/'results.json').write_text(json.dumps(results,indent=2)+'\n')
    else: assert json.loads((a.dataset/'results.json').read_text())==results
    print(json.dumps(summaries,indent=2))
    if a.validate_only: return
    import matplotlib;matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    historical=json.loads((Path(__file__).resolve().parents[1]/'benchmarks/scheduling-2026-09-30/results.json').read_text())
    fig,axes=plt.subplots(3,2,figsize=(12,11))
    colors={'global':'#245ea5','component':'#6f9ccb','auto':'#209a8d','candidate':'#5b3998','cpu':'#df7c26'}
    for i,(scene,title) in enumerate([('falling-cubes','50,000 falling cubes'),('mixed-stacks','4,096 independent-group bodies'),('mixed-topology','8,192-body pile + 4,096 group bodies')]):
        groups=[]
        cpu=[r for r in historical['rows'] if r['phase']=='baseline' and r['scene']==scene and r['schedule']=='cpu']
        if cpu: groups.append(('cpu','CPU (prior experiment)',cpu))
        for schedule in ['global','component','auto','candidate']:
            phase='confirmation' if schedule in ['auto','candidate'] and any(r['phase']=='confirmation' for r in rows) else 'baseline'
            selected=[r for r in rows if r['scene']==scene and r['phase']==phase and r['schedule']==schedule]
            if not selected and schedule=='candidate':
                selected=[r for r in rows if r['scene']==scene and r['phase'].startswith('pilot-') and r['schedule']==schedule]
            if selected: groups.append((schedule,('GPU split (rejected)' if schedule=='candidate' else 'GPU '+schedule)+(' (pilot)' if selected[0].get('phase','').startswith('pilot-') else ''),selected))
        for ax,key,ylabel in zip(axes[i],['mean_ms','p95_ms'],['Median trial mean (ms)','Median trial p95 (ms)']):
            values=[statistics.median(r[key] for r in group) for _,_,group in groups]
            ax.bar(range(len(groups)),values,color=[colors[s] for s,_,_ in groups])
            for x,(_,_,group) in enumerate(groups):
                ax.scatter([x]*len(group),[r[key] for r in group],color='#202020',s=16,zorder=3)
            ax.set_xticks(range(len(groups)),[label for _,label,_ in groups],rotation=17,ha='right')
            ax.set_ylabel(ylabel);ax.set_title(title);ax.grid(axis='y',alpha=.2)
            for x,v in enumerate(values): ax.text(x,v,f'{v:.3f}',ha='center',va='bottom',fontsize=9)
            ax.set_ylim(0,max(values)*1.2)
    fig.suptitle('i9-9900K / RTX 4070 SUPER • native Vulkan\n4 substeps, dt 1/60, no sleep; 90 warmup + 240 completed steps; dots = all trials',fontsize=12)
    fig.tight_layout(rect=[0,.04,1,.96])
    fig.text(.5,.005,'CPU orange: preserved prior same-machine controls, not fresh mixed experiment runs. GPU controls blue/teal; candidate purple.',ha='center',fontsize=9)
    for extension in ['png','svg']:
        fig.savefig(a.dataset/f'comparison.{extension}',dpi=160)
    plt.close(fig)
    component=[(k,v) for k,v in phases.items() if '-component-' in k]
    fig,axes=plt.subplots(1,len(component),figsize=(14,4.5))
    for ax,(name,data) in zip(axes,component):
        names=['Reset','Count','Offsets','Color offsets','Scatter','Small solve','Large solve']
        ax.barh(names,data['gpu_phase_mean_ms'],color=colors['global']);ax.set_xscale('log')
        ax.set_xlim(right=max(data['gpu_phase_mean_ms'])*3)
        ax.set_xlabel('Mean diagnostic GPU ms (log scale)');ax.set_title(Path(name).stem.replace('-component-1',''))
        for y,v in enumerate(data['gpu_phase_mean_ms']): ax.text(v,y,f' {v:.3f}',va='center',fontsize=9)
    fig.tight_layout()
    for extension in ['png','svg']: fig.savefig(a.dataset/f'component-phases.{extension}',dpi=160)
    plt.close(fig)
    for svg in a.dataset.glob('*.svg'):
        svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines())+'\n')

if __name__=='__main__': main()
