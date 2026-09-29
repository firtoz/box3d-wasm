#!/usr/bin/env python3
"""Verify checked-in machine datasets and rerender CPU/GPU scaling comparisons."""
import argparse
import csv
import json
from pathlib import Path
import math
from cube_machine_data import ROOT, read_dataset, summaries


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--data-dir',type=Path,default=ROOT/'benchmarks/machines')
    p.add_argument('--output-dir',type=Path)
    p.add_argument('--validate-only',action='store_true')
    p.add_argument('--allow-protocol-differences',action='store_true',help='Show explicitly different protocols with a warning in the chart')
    a=p.parse_args()
    datasets=[read_dataset(path)[0] for path in sorted(a.data_dir.glob('*.json'))]
    assert datasets,'no machine datasets'
    # Counts select plotted workloads, not measurement conditions. A 50k-only
    # machine can be compared with the same point in a larger sweep.
    protocols={json.dumps({k:v for k,v in d['protocol'].items() if k!='counts'},sort_keys=True) for d in datasets}
    assert len(protocols)==1 or a.allow_protocol_differences,'protocols differ; inspect data before using --allow-protocol-differences'
    if a.validate_only:
        print(f'Validated {len(datasets)} machines, raw hashes and all successful trial metrics')
        return
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from matplotlib.ticker import FuncFormatter,NullFormatter
    out=a.output_dir or a.data_dir/'charts';out.mkdir(parents=True,exist_ok=True)
    fig,axes=plt.subplots(2,2,figsize=(14,10),layout='constrained')
    all_counts=sorted({n for d in datasets for n in d['protocol']['counts']})
    single_count=len(all_counts)==1
    panels=[('physics','Completed physics','steps/s'),('direct','Same direct renderer, CPU/GPU physics','FPS')]
    table=[]
    for m,data in enumerate(datasets):
        color=plt.get_cmap('tab10')(m%10)
        rows=summaries(data)
        for row in rows:table.append({'machine':data['machine_id'],'label':data['label'],**row})
        for col,(prefix,title,unit) in enumerate(panels):
            found=False
            for engine,style,marker in [('cpu','-','o'),('gpu','--','s')]:
                series={r['count']:r for r in rows if r['mode']==prefix+'-'+engine}
                if not series:continue
                found=True
                # Missing or incomplete counts break curves; never extrapolate a stop.
                counts=data['protocol']['counts'];nan=float('nan')
                rates=[series[n]['rate'] if n in series else nan for n in counts]
                lo=[series[n]['rate_low'] if n in series else nan for n in counts]
                hi=[series[n]['rate_high'] if n in series else nan for n in counts]
                if single_count:
                    x=m*3+(engine=='gpu')
                    for row,key in [(0,'rate'),(1,'p95_ms')]:
                        value=series[counts[0]][key]
                        axes[row,col].bar(x,value,color=color,alpha=1 if engine=='gpu' else .5)
                        axes[row,col].annotate(f'{value:.2f}',(x,value),xytext=(0,6),textcoords='offset points',ha='center')
                    axes[0,col].errorbar(x,rates[0],yerr=[[rates[0]-lo[0]],[hi[0]-rates[0]]],fmt='none',ecolor='black',capsize=4)
                    continue
                axes[0,col].plot(counts,rates,color=color,linestyle=style,marker=marker,markersize=4,label=f"{data['label']} · {engine.upper()}")
                axes[0,col].fill_between(counts,lo,hi,color=color,alpha=.10)
                axes[1,col].plot(counts,[series[n]['p95_ms'] if n in series else nan for n in counts],color=color,linestyle=style,marker=marker,markersize=4)
    for col,(prefix,title,unit) in enumerate(panels):
        for row in [0,1]:
            ax=axes[row,col]
            if not single_count:ax.set_xscale('log');ax.set_yscale('log')
            ax.grid(alpha=.2,which='major');ax.set_axisbelow(True)
            ax.yaxis.set_major_formatter(FuncFormatter(lambda v,pos:f'{v:g}'))
            ax.yaxis.set_minor_formatter(NullFormatter())
            ticks=all_counts
            if single_count:
                ax.set_xticks([m*3+i for m in range(len(datasets)) for i in [0,1]],
                    [d['label']+'\n'+engine for d in datasets for engine in ['CPU','GPU']],fontsize=9)
                ax.set_ylim(bottom=0,top=ax.get_ylim()[1]*1.12)
            else:ax.set_xticks(ticks,[f'{n/1000:g}k' if n>=1000 else str(n) for n in ticks],rotation=35)
            ax.xaxis.set_minor_formatter(NullFormatter())
            ax.set_xlabel(f'{ticks[0]:,} dynamic cubes (+ one static floor)' if single_count else 'Dynamic cubes (+ one static floor)')
            if not ax.lines and not ax.patches:ax.text(.5,.5,'No valid measurements yet',transform=ax.transAxes,ha='center',fontsize=14)
        axes[0,col].set_title(title);axes[0,col].set_ylabel(unit+' (higher is better)')
        axes[1,col].set_title('Completed step p95' if prefix=='physics' else 'Frame cadence p95')
        axes[1,col].set_ylabel('Milliseconds (lower is better)')
        if axes[0,col].lines and not single_count:axes[0,col].legend(fontsize=8,loc='best')
        for rate in [30,60]:
            axes[0,col].axhline(rate,color='#888',linestyle=':',linewidth=.7)
            axes[1,col].axhline(1000/rate,color='#888',linestyle=':',linewidth=.7)
    note='PROTOCOLS DIFFER — see metadata; differences cannot be attributed only to hardware.' if len(protocols)>1 else 'Shared measurement protocol across machines.'
    proto=datasets[0]['protocol']
    missing=' Partial datasets: '+', '.join(d['label'] for d in datasets if d['status']!='complete') if any(d['status']!='complete' for d in datasets) else ''
    maximum=max(n for d in datasets for n in d['protocol']['counts'])
    fig.suptitle(f'Box3D CPU vs GPU · cubes up to {maximum:,}',fontsize=18)
    fig.supxlabel(f"{proto['trials']} trials · {proto['warmup']} warmup + {proto['timed']} timed steps · 4 substeps · sleep off · {proto['workers']} CPU workers\n"
        f"{proto['backend']} / {proto['gpu_solver']} solver · renderer {proto['width']}×{proto['height']}, unpaced. Rate = 1000 / median trial mean ms; bands/whiskers = full trial range.\n"
        'Latency = median trial p95, not pooled p95. Physics steps/s is not display FPS. '+note+missing,fontsize=9)
    for ext in ['png','svg']:fig.savefig(out/('cube-scaling.'+ext),dpi=160,bbox_inches='tight')
    svg=out/'cube-scaling.svg'
    svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines())+'\n')
    with (out/'cube-scaling.csv').open('w') as f:
        writer=csv.DictWriter(f,lineterminator='\n',fieldnames=list(table[0]) if table else ['machine']);writer.writeheader();writer.writerows(table)
    lines=['# Cube scaling measurements','',note,'','| Machine | Path | Cubes | Rate (/s) | p50 ms | p95 ms |','| --- | --- | ---: | ---: | ---: | ---: |']
    for r in table:lines.append(f"| {r['label']} | {r['mode']} | {r['count']:,} | {r['rate']:.2f} | {r['p50_ms']:.3f} | {r['p95_ms']:.3f} |")
    lines+=['','Rates for physics paths are completed steps/s; direct-renderer rates are FPS. Missing points are not extrapolated.','']
    for d in datasets:
        revisions=', '.join(sorted({b['git'][:12] for b in d['batches'].values()}))
        lines += [f"- **{d['label']}**: {d['status']}; runner checkout {revisions}; backend {d['protocol']['backend']}; stops: `{json.dumps(d['stopped'],sort_keys=True)}`."]
    (out/'README.md').write_text('\n'.join(lines)+'\n')
    print(out/'cube-scaling.png')

if __name__=='__main__':main()
