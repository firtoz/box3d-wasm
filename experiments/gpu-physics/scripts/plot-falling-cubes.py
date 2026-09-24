#!/usr/bin/env python3
"""Publish a compact auditable summary and chart from a falling-cubes sweep.
Requires matplotlib (plotting only; benchmark runner has no Python dependencies).
FPS = reciprocal of median trial mean frame time. p50/p95 remain milliseconds.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import statistics
import runpy
from types import SimpleNamespace
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, NullFormatter

MODES=['physics-cpu','physics-gpu','sokol-cpu','sokol-gpu','direct-cpu','direct-gpu']

def summarize(folder):
    manifest=json.loads((folder/'manifest.json').read_text())
    trials=json.loads((folder/'trials.json').read_text())
    stopped=json.loads((folder/'stopped.json').read_text())
    validate=runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))['metrics']
    args=SimpleNamespace(**manifest['arguments'])
    identities=[(r['count'],r['mode'],r['trial']) for r in trials]
    assert len(identities)==len(set(identities)), 'duplicate trial identity'
    for trial in trials:
        if trial['status']=='ok':
            path=folder/f"{trial['count']}-{trial['mode']}-{trial['trial']}.json"
            measured=validate(json.loads(path.read_text()),trial['mode'],path,args,trial['count'])
            for key in ['mean_ms','p50_ms','p95_ms']:
                assert math.isclose(measured[key],trial[key],rel_tol=1e-9,abs_tol=1e-9), (path,key,'summary disagrees with raw data')
    rows=[]
    for count in manifest['arguments']['counts']:
        for mode in MODES:
            group=[r for r in trials if r['count']==count and r['mode']==mode and r['status']=='ok']
            if len(group)!=manifest['arguments']['trials']: continue
            means=[r['mean_ms'] for r in group]
            rows.append(dict(count=count,mode=mode,fps=1000/statistics.median(means),
                fps_min=1000/max(means),fps_max=1000/min(means),
                p50_ms=statistics.median(r['p50_ms'] for r in group),
                p95_ms=statistics.median(r['p95_ms'] for r in group),
                trial_means_ms=means,framebuffer=group[0].get('framebuffer')))
    return dict(workload=manifest['workload'],manifest=manifest,results=rows,stopped=stopped,
        raw_sha256={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(folder.glob('*.json'))},
        **derive(rows))

def derive(rows):
    thresholds={}
    for mode in MODES:
        series=[r for r in rows if r['mode']==mode]
        thresholds[mode]={}
        for fps in (60,30,10):
            below=next((i for i,r in enumerate(series) if r['fps']<=fps),None)
            thresholds[mode][str(fps)]=dict(last_above=series[below-1]['count'] if below and below>0 else None,
                first_at_or_below=series[below]['count'] if below is not None else None,
                last_tested=series[-1]['count'] if series else None)
    variability=[dict(count=r['count'],mode=r['mode'],slowest_over_fastest=max(r['trial_means_ms'])/min(r['trial_means_ms']))
        for r in rows if max(r['trial_means_ms'])/min(r['trial_means_ms'])>1.5]
    crossovers={}
    for prefix in ['physics','sokol','direct']:
        paired=[]
        for cpu in [r for r in rows if r['mode']==prefix+'-cpu']:
            gpu=next((r for r in rows if r['mode']==prefix+'-gpu' and r['count']==cpu['count']),None)
            if gpu:
                paired.append(dict(count=cpu['count'],gpu_over_cpu=gpu['fps']/cpu['fps'],
                    trial_ranges_overlap=not(gpu['fps_max']<cpu['fps_min'] or cpu['fps_max']<gpu['fps_min']),
                    trial_gpu_over_cpu=[c/g for c,g in zip(cpu['trial_means_ms'],gpu['trial_means_ms'])]))
        crossovers[prefix]=dict(paired=paired,gpu_faster_counts=[r['count'] for r in paired if r['gpu_over_cpu']>1],
            gpu_20pct_faster_counts=[r['count'] for r in paired if r['gpu_over_cpu']>=1.2],
            all_trials_20pct_faster_counts=[r['count'] for r in paired if all(v>=1.2 for v in r['trial_gpu_over_cpu'])])
    return dict(variability_flags=variability,crossovers=crossovers,thresholds=thresholds)

def combine(folders):
    reports=[summarize(folder) for folder in folders]
    report=reports[0]
    hashes={folders[0].name+'/'+k:v for k,v in report['raw_sha256'].items()}
    additional=[]
    for folder,extra in zip(folders[1:],reports[1:]):
        for key in ['workload']:
            assert report[key]==extra[key], key
        for key in ['binaries','environment']:
            assert report['manifest'][key]==extra['manifest'][key], key
        for key in ['trials','warmup','timed','workers','adapter','width','height']:
            assert report['manifest']['arguments'][key]==extra['manifest']['arguments'][key], key
        existing={(r['count'],r['mode']) for r in report['results']}
        assert not existing.intersection((r['count'],r['mode']) for r in extra['results']), 'overlapping count/mode datasets'
        report['results'].extend(extra['results'])
        report['stopped'].update(extra['stopped'])
        additional.append(extra['manifest'])
        hashes.update({folder.name+'/'+k:v for k,v in extra['raw_sha256'].items()})
    report['results'].sort(key=lambda r:(r['count'],r['mode']))
    sizes={tuple(r['framebuffer']) for r in report['results'] if r['framebuffer']}
    assert len(sizes)<=1, 'mismatched framebuffers'
    report.update(derive(report['results']),additional_manifests=additional,raw_sha256=hashes)
    return report

def plot_latencies(report, output, title, panels, ticks, scope_note):
    """Expose tail latency separately from reciprocal mean-time throughput."""
    fig,axes=plt.subplots(len(panels),1,figsize=(11,4.8+3.1*(len(panels)-1)),sharex=True,layout='constrained')
    if len(panels)==1: axes=[axes]
    fig.suptitle('Falling cubes • p50 / p95 latency\n'+title,fontsize=13,fontweight='bold')
    for ax,(prefix,panel_title) in zip(axes,panels):
        for engine,color in [('cpu','#2266bb'),('gpu','#c65324')]:
            rows=[r for r in report['results'] if r['mode']==prefix+'-'+engine]
            if not rows: continue
            for key,style,marker in [('p50_ms','-','o'),('p95_ms','--','^')]:
                ax.plot([r['count'] for r in rows],[r[key] for r in rows],
                    color=color,linestyle=style,marker=marker,markersize=3,
                    label=f"{engine.upper()} {key[:3]}")
        for rate in (60,30,10):
            budget=1000/rate
            ax.axhline(budget,color='#888888',linewidth=.7,linestyle=':')
            ax.text(1.002,budget,f'{rate}/s',transform=ax.get_yaxis_transform(),va='center',fontsize=8,color='#555555')
        ax.set(xscale='log',yscale='log',ylabel='Milliseconds (lower is better)',
            title=panel_title.replace('throughput','step latency').replace('full frame cadence','frame cadence latency'))
        ax.yaxis.set_major_formatter(FuncFormatter(lambda x,pos:f'{x:g}'))
        ax.yaxis.set_minor_formatter(NullFormatter())
        ax.grid(alpha=.18);ax.legend(loc='upper left',ncols=2,fontsize=8)
    axes[-1].set_xlabel('Dynamic 1 m cubes (one additional static floor)')
    axes[-1].set_xticks(ticks,[f'{x:,}' for x in ticks],rotation=45,ha='right')
    axes[-1].xaxis.set_minor_formatter(NullFormatter())
    args=report['manifest']['arguments']
    sizes={tuple(r['framebuffer']) for r in report['results'] if r['framebuffer']}
    rendering=f" • framebuffer {', '.join(f'{w}×{h}' for w,h in sorted(sizes))}" if sizes else ''
    fig.supxlabel(f"Medians of per-trial percentiles across {args['trials']} trials; not pooled percentiles\n"
        +f"dt 1/60 s • 4 substeps • sleep off • steps {args['warmup']+1}–{args['warmup']+args['timed']} • dotted lines are time budgets, not average FPS\n"
        +f"{args['workers']} CPU workers{rendering} • {scope_note}",fontsize=9)
    for suffix in ('.svg','.png'):
        path=output.with_name(output.name+'-timings').with_suffix(suffix)
        fig.savefig(path,dpi=150)
        if suffix=='.svg': path.write_text('\n'.join(line.rstrip() for line in path.read_text().splitlines())+'\n')
    plt.close(fig)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('input',type=Path)
    p.add_argument('output',type=Path,help='file stem for .json, .svg and .png')
    p.add_argument('--additional-input',type=Path,action='append',default=[],help='Non-overlapping continuation counts with identical binaries and settings')
    p.add_argument('--title',required=True,help='Exact hardware/driver label')
    p.add_argument('--scope-note',default='stop at ≤10 FPS or capacity/validation limit',help='Measurement coverage shown in the chart footer')
    a=p.parse_args(); report=combine([a.input]+a.additional_input)
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
    plt.rcParams.update({'font.family':'DejaVu Sans','font.size':10,'svg.fonttype':'none'})
    panels=[(prefix,title) for prefix,title in zip(['physics','sokol','direct'],
        ['Completed physics throughput (no rendering)','Sokol sample app • full frame cadence','Direct instanced renderer • full frame cadence'])
        if any(r['mode'].startswith(prefix+'-') for r in report['results'])]
    assert panels, 'no complete measurement series'
    fig,axes=plt.subplots(len(panels),1,figsize=(11,4.8+3.1*(len(panels)-1)),sharex=True,layout='constrained')
    if len(panels)==1: axes=[axes]
    fig.suptitle('Falling cubes • '+a.title,fontsize=14,fontweight='bold')
    for ax,(prefix,title) in zip(axes,panels):
        for engine,color in [('cpu','#2266bb'),('gpu','#c65324')]:
            rows=[r for r in report['results'] if r['mode']==prefix+'-'+engine]
            if not rows: continue
            x=[r['count'] for r in rows];y=[r['fps'] for r in rows]
            ax.plot(x,y,'o-',color=color,label='Box3D CPU' if engine=='cpu' else 'GPU physics',markersize=4)
            ax.fill_between(x,[r['fps_min'] for r in rows],[r['fps_max'] for r in rows],color=color,alpha=.15)
        gpu_stop=report['stopped'].get(prefix+'-gpu',{})
        if gpu_stop and gpu_stop['reason']!='10 FPS threshold':
            ax.axvline(gpu_stop['count'],color='#c65324',linestyle=':',linewidth=1)
            ax.text(gpu_stop['count'],.76,' GPU validation failure',transform=ax.get_xaxis_transform(),
                rotation=90,va='top',ha='right',fontsize=8,color='#a34520')
        cpu_stop=report['stopped'].get(prefix+'-cpu',{})
        if 'max_dynamic_cubes' in cpu_stop:
            ax.axvline(cpu_stop['max_dynamic_cubes'],color='#2266bb',linestyle=':',linewidth=1)
            ax.text(cpu_stop['max_dynamic_cubes'],.78,' CPU viewer body limit',transform=ax.get_xaxis_transform(),
                rotation=90,va='top',ha='right',fontsize=8,color='#2266bb')
        for fps in (60,30,10):
            ax.axhline(fps,color='#888888',linewidth=.7,linestyle='--')
            ax.text(1.002,fps,str(fps),transform=ax.get_yaxis_transform(),va='center',fontsize=9,color='#555555')
        ax.set(xscale='log',yscale='log',ylabel='Steps/s' if prefix=='physics' else 'Frames/s',title=title)
        ax.grid(alpha=.18);ax.legend(loc='upper right',fontsize=9)
        if ax.get_ylim()[1] <= 100:
            low,high=ax.get_ylim()
            ax.set_yticks([v for v in [10,20,30,40,60,100] if low<=v<=high])
            ax.yaxis.set_minor_formatter(NullFormatter())
        ax.yaxis.set_major_formatter(FuncFormatter(lambda x,pos:f'{x:g}'))
    axes[-1].set_xlabel('Dynamic 1 m cubes (one additional static floor)')
    ticks=sorted({r['count'] for r in report['results']})
    labels=[x for x in [5,10,50,100,200,400,1000,2000,5000,10000,20000,50000,100000,150000,400000,1000000] if ticks[0]<=x<=ticks[-1]]
    if len(ticks)<=8: labels=ticks
    axes[-1].set_xticks(labels,[f'{x:,}' for x in labels],rotation=45,ha='right')
    axes[-1].xaxis.set_minor_formatter(NullFormatter())
    args=report['manifest']['arguments']; sizes={tuple(r['framebuffer']) for r in report['results'] if r['framebuffer']}
    rendering=f"framebuffer {', '.join(f'{w}×{h}' for w,h in sizes)} • uncapped • " if sizes else ''
    fig.supxlabel(f"{args['trials']} trials • shaded trial range • dt 1/60 s; 4 substeps • sleep off • steps {args['warmup']+1}–{args['warmup']+args['timed']}\n"
        +f"{args['workers']} CPU workers • {rendering}{a.scope_note}",fontsize=9)
    fig.savefig(a.output.with_suffix('.svg'))
    fig.savefig(a.output.with_suffix('.png'),dpi=150)
    svg=a.output.with_suffix('.svg')
    svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines())+'\n')
    lines=['| Cubes | Physics CPU / GPU | Sokol CPU / GPU | Direct CPU / GPU |','|---:|---:|---:|---:|']
    for count in ticks:
        values=[]
        for prefix in ['physics','sokol','direct']:
            vals=[]
            for engine in ['cpu','gpu']:
                row=next((r for r in report['results'] if r['count']==count and r['mode']==prefix+'-'+engine),None)
                vals.append(f"{row['fps']:,.1f}" if row else '—')
            values.append(' / '.join(vals))
        lines.append('| '+str(count)+' | '+' | '.join(values)+' |')
    a.output.with_suffix('.md').write_text('\n'.join(lines)+'\n')
    timing=['| Cubes | '+ ' | '.join(MODES)+' |','|---:|'+'---:|'*len(MODES)]
    for count in ticks:
        cells=[]
        for mode in MODES:
            row=next((r for r in report['results'] if r['count']==count and r['mode']==mode),None)
            cells.append(f"{row['p50_ms']:.2f} / {row['p95_ms']:.2f}" if row else '—')
        timing.append('| '+str(count)+' | '+' | '.join(cells)+' |')
    a.output.with_name(a.output.name+'-timings.md').write_text('p50 / p95 milliseconds; median across repeated trials.\n\n'+'\n'.join(timing)+'\n')
    plot_latencies(report,a.output,a.title,panels,labels,a.scope_note)
    print('\n'.join(lines));print(json.dumps(report['thresholds'],indent=2))

if __name__=='__main__':main()
