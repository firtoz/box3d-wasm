#!/usr/bin/env python3
"""Hash-check portable data and plot completed steps plus diagnostic phases."""
import argparse,gzip,hashlib,json,statistics
from pathlib import Path

def save_figure(fig, directory, stem):
    for extension in ['png','svg']:
        path=directory/f'{stem}.{extension}'
        fig.savefig(path,dpi=160)
        if extension=='svg':
            path.write_text('\n'.join(line.rstrip() for line in path.read_text().splitlines())+'\n')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('dataset',type=Path);p.add_argument('--validate-only',action='store_true');a=p.parse_args()
    d=json.loads((a.dataset/'results.json').read_text());packed=(a.dataset/'raw.json.gz').read_bytes()
    assert hashlib.sha256(packed).hexdigest()==d['raw_sha256'];bundle=json.loads(gzip.decompress(packed));assert bundle['schema']==1
    for row in d['rows']:
        raw=json.loads(bundle['files'][row['raw']])
        samples=raw['scenes'][row['scene']]['wall_samples_ms'] if row['schedule']=='cpu' else raw['raw_runs'][0]['completed_step_ms']
        assert abs(statistics.mean(samples)-row['mean_ms'])<1e-9
        reference=raw['scenes'][row['scene']] if row['schedule']=='cpu' else raw['completed_step']
        assert reference['wall_p95_ms' if row['schedule']=='cpu' else 'p95_ms']==row['p95_ms']
        assert reference['wall_p50_ms' if row['schedule']=='cpu' else 'p50_ms']==row['p50_ms']
        assert abs(1000/statistics.mean(samples)-row['steps_per_second'])<1e-9
    for scene in ['falling-cubes','mixed-stacks']:
        for phase,schedule in [('baseline','global'),('baseline','component'),('baseline','cpu'),('confirmation','global'),('confirmation','auto')]:
            assert sum(row['phase']==phase and row['scene']==scene and row['schedule']==schedule for row in d['rows'])==3, (phase,scene,schedule)
    if a.validate_only: return
    gpu_row=next(row for row in d['rows'] if row['schedule']!='cpu')
    adapter=json.loads(bundle['files'][gpu_row['raw']])['adapter']
    manifest=json.loads(bundle['files'][gpu_row['manifest']])
    cpu_model=next((line.split(':',1)[1].strip() for line in manifest['cpu'].splitlines() if line.startswith('Model name:')),'CPU model in manifest')
    import matplotlib;matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    colors={'cpu':'#e08024','global':'#315eaa','component':'#74a9cf','auto':'#168b6a'}
    fig,axes=plt.subplots(2,3,figsize=(13,7.5))
    labels=['CPU','GPU global','GPU component','GPU automatic']
    for i,(scene,title) in enumerate([('falling-cubes','50,000 falling cubes'),('mixed-stacks','4,096 independent-group bodies')]):
        groups=[]
        for schedule in ['cpu','global','component','auto']:
            phase='confirmation' if schedule in ['global','auto'] else 'baseline'
            rows=[r for r in d['rows'] if r['phase']==phase and r['scene']==scene and r['schedule']==schedule]
            assert len(rows)==3,(phase,scene,schedule,len(rows));groups.append(rows)
        for j,(key,ylabel) in enumerate([('steps_per_second','Completed steps/s'),('mean_ms','Completed-step mean (ms)'),('p95_ms','Completed-step p95 (ms)')]):
            ax=axes[i,j];med=[statistics.median(r[key] for r in rows) for rows in groups]
            ax.bar(range(4),med,color=list(colors.values()));
            for x,rows in enumerate(groups):ax.scatter([x]*3,[r[key] for r in rows],color='#202020',s=14,zorder=3)
            ax.set_ylim(0,max(r[key] for rows in groups for r in rows)*1.18)
            ax.set_xticks(range(4),labels,rotation=15,ha='right');ax.set_ylabel(ylabel);ax.set_title(title);ax.grid(axis='y',alpha=.2)
            for x,v in enumerate(med):ax.text(x,v,f'{v:.2f}',ha='center',va='bottom',fontsize=9)
    fig.suptitle(f'Native Vulkan • {adapter} / {cpu_model}\nFour substeps, no sleep; 3 fresh processes; 90 warmup + 240 timed; dots show all trials',fontsize=11)
    fig.tight_layout(rect=[0,.03,1,.95])
    fig.text(.5,.005,'Global/automatic: alternating confirmation. Component/CPU: initial controls. Bars: median trial values; dots: all three trials.',ha='center',fontsize=9)
    save_figure(fig,a.dataset,'comparison')
    plt.close(fig)
    if not d['diagnostic_component_phases']: return
    fig,axes=plt.subplots(1,2,figsize=(12,4))
    for ax,(name,phase) in zip(axes,d['diagnostic_component_phases'].items()):
        ax.barh(phase['names'],phase['mean_ms'],color='#315eaa');ax.set_xlim(0,max(phase['mean_ms'])*1.2)
        for y,value in enumerate(phase['mean_ms']): ax.text(value,y,f' {value:.3f}',va='center',fontsize=9)
        ax.set_xlabel('Mean GPU phase ms (diagnostic only)');ax.set_title(name.split('/')[-1].replace('-component.log',''));ax.grid(axis='x',alpha=.2)
    fig.suptitle('Component scheduling attribution • separate diagnostic build/window',fontsize=12);fig.tight_layout()
    save_figure(fig,a.dataset,'component-phases')
    plt.close(fig)

if __name__=='__main__':main()
