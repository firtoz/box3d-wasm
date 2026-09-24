#!/usr/bin/env python3
"""Publish a compact auditable summary and chart from a falling-cubes sweep.
Requires matplotlib (plotting only; benchmark runner has no Python dependencies).
FPS = reciprocal of median trial mean frame time. p50/p95 remain milliseconds.
"""
import argparse
import hashlib
import json
from pathlib import Path
import statistics
import runpy
from types import SimpleNamespace
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

MODES=['physics-cpu','physics-gpu','sokol-cpu','sokol-gpu','direct-cpu','direct-gpu']

def summarize(folder):
    manifest=json.loads((folder/'manifest.json').read_text())
    trials=json.loads((folder/'trials.json').read_text())
    stopped=json.loads((folder/'stopped.json').read_text())
    validate=runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))['metrics']
    args=SimpleNamespace(**manifest['arguments'])
    for trial in trials:
        if trial['status']=='ok':
            path=folder/f"{trial['count']}-{trial['mode']}-{trial['trial']}.json"
            validate(json.loads(path.read_text()),trial['mode'],path,args,trial['count'])
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
    thresholds={}
    for mode in MODES:
        series=[r for r in rows if r['mode']==mode]
        thresholds[mode]={}
        for fps in (60,30,10):
            below=next((i for i,r in enumerate(series) if r['fps']<=fps),None)
            thresholds[mode][str(fps)]=dict(last_above=series[below-1]['count'] if below and below>0 else None,
                first_at_or_below=series[below]['count'] if below is not None else None,
                last_tested=series[-1]['count'] if series else None)
    return dict(workload=manifest['workload'],manifest=manifest,results=rows,thresholds=thresholds,stopped=stopped,
        raw_sha256={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(folder.glob('*.json'))})

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('input',type=Path)
    p.add_argument('output',type=Path,help='file stem for .json, .svg and .png')
    p.add_argument('--title',required=True,help='Exact hardware/driver label')
    a=p.parse_args(); report=summarize(a.input)
    a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
    plt.rcParams.update({'font.family':'DejaVu Sans','font.size':10,'svg.fonttype':'none'})
    fig,axes=plt.subplots(3,1,figsize=(11,11),sharex=True,layout='constrained')
    fig.suptitle('Falling cubes • '+a.title,fontsize=15,fontweight='bold')
    for ax,prefix,title in zip(axes,['physics','sokol','direct'],['Completed physics throughput (no rendering)','Sokol sample app • full frame cadence','Direct instanced renderer • full frame cadence']):
        for engine,color in [('cpu','#2266bb'),('gpu','#c65324')]:
            rows=[r for r in report['results'] if r['mode']==prefix+'-'+engine]
            if not rows: continue
            x=[r['count'] for r in rows];y=[r['fps'] for r in rows]
            ax.plot(x,y,'o-',color=color,label='Box3D CPU' if engine=='cpu' else 'GPU physics',markersize=4)
            ax.fill_between(x,[r['fps_min'] for r in rows],[r['fps_max'] for r in rows],color=color,alpha=.15)
        for fps in (60,30,10):
            ax.axhline(fps,color='#888888',linewidth=.7,linestyle='--')
            ax.text(1.002,fps,str(fps),transform=ax.get_yaxis_transform(),va='center',fontsize=9,color='#555555')
        ax.set(xscale='log',yscale='log',ylabel='Steps/s' if prefix=='physics' else 'Frames/s',title=title)
        ax.grid(alpha=.18);ax.legend(loc='upper right',fontsize=9)
        ax.yaxis.set_major_formatter(FuncFormatter(lambda x,pos:f'{x:g}'))
    axes[-1].set_xlabel('Dynamic 1 m cubes (one additional static floor)')
    ticks=sorted({r['count'] for r in report['results']})
    axes[-1].set_xticks(ticks,[f'{x:,}' for x in ticks],rotation=45,ha='right')
    args=report['manifest']['arguments']; sizes={tuple(r['framebuffer']) for r in report['results'] if r['framebuffer']}
    fig.supxlabel(f"{args['trials']} trials • shaded trial range • dt 1/60 s; 4 substeps • sleep off • steps {args['warmup']+1}–{args['warmup']+args['timed']}\n"
        +f"{args['workers']} CPU workers • framebuffer {', '.join(f'{w}×{h}' for w,h in sizes)} • uncapped • stop each path at ≤10 FPS or invalid result",fontsize=9)
    fig.savefig(a.output.with_suffix('.svg'))
    fig.savefig(a.output.with_suffix('.png'),dpi=150)
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
    print('\n'.join(lines));print(json.dumps(report['thresholds'],indent=2))

if __name__=='__main__':main()
