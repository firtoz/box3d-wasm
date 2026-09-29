#!/usr/bin/env python3
"""Reproduce the 50k solver-setting investigation from verified portable datasets."""
import json
from pathlib import Path
from cube_machine_data import ROOT, read_dataset, summaries

BASE=ROOT/'benchmarks/machines'
CURRENT='desktop-i9-9900k-rtx4070-super-global-50k-2026-09-29.json'
COMPONENT='component-baseline/desktop-i9-9900k-rtx4070-super-2026-09-29.json'
HIST_GLOBAL='history/historical-49b024f-global-50k.json'
HIST_COMPONENT='history/historical-49b024f-component-50k.json'
variants=[('CPU Box3D',CURRENT,'physics-cpu'),
          ('49b024f\ncomponent',HIST_COMPONENT,'physics-gpu'),
          ('d98f2f3\ncomponent',COMPONENT,'physics-gpu'),
          ('49b024f\nglobal',HIST_GLOBAL,'physics-gpu'),
          ('d98f2f3\nglobal',CURRENT,'physics-gpu')]
rows=[]
builds=json.loads((ROOT/'benchmarks/cube-regression/build-revisions.json').read_text())['builds']
for label,file,mode in variants:
    data,bundle=read_dataset(BASE/file)
    if mode=='physics-gpu':
        expected='49b024f' if file.startswith('history/') else 'd98f2f3'
        for trial in data['trials']:
            if trial['mode']==mode and trial['count']==50000 and trial['status']=='ok':
                assert data['batches'][trial['batch']]['binaries']['gpu']['sha256']==builds[expected]['binary_sha256'], 'binary build provenance mismatch'
    row=next(r for r in summaries(data) if r['count']==50000 and r['mode']==mode)
    rows.append(dict(label=label.replace('\n',' / '),dataset=file,trials=data['protocol']['trials'],**row))
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
fig,axes=plt.subplots(1,2,figsize=(12,5),layout='constrained')
colors=['#777777','#dc9c48','#ba6c22','#5599ce','#17659e']
for ax,key,title,unit in [(axes[0],'rate','Completed physics throughput','Steps/s (higher is better)'),
                          (axes[1],'p95_ms','Completed step p95','Milliseconds (lower is better)')]:
    ax.bar(range(len(rows)),[r[key] for r in rows],color=colors)
    for i,r in enumerate(rows):ax.annotate(f"{r[key]:.1f}",(i,r[key]),xytext=(0,5),textcoords='offset points',ha='center')
    ax.set_xticks(range(len(rows)),[v[0] for v in variants],fontsize=9)
    ax.set_title(title);ax.set_ylabel(unit);ax.grid(axis='y',alpha=.2);ax.set_axisbelow(True)
    ax.set_ylim(0,max(r[key] for r in rows)*1.18)
axes[0].errorbar(range(len(rows)),[r['rate'] for r in rows],
    yerr=[[r['rate']-r['rate_low'] for r in rows],[r['rate_high']-r['rate'] for r in rows]],
    fmt='none',ecolor='black',capsize=4)
fig.suptitle('50,000 cubes · same i9-9900K / RTX 4070 SUPER',fontsize=16)
fig.supxlabel('Native cached Vulkan · 90 warmup + 240 timed steps · 4 substeps · sleep off · 8 CPU workers\n'
    'Three trials except historical component (one diagnostic trial). Bars = median trial rates; whiskers = full trial range.\n'
    'p95 = median trial p95. These are physics rates, not rendered FPS.',fontsize=9)
out=ROOT/'benchmarks/cube-regression';out.mkdir(parents=True,exist_ok=True)
for ext in ['png','svg']:fig.savefig(out/('50k-solver-comparison.'+ext),dpi=160)
svg=out/'50k-solver-comparison.svg'
svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines())+'\n')
(out/'50k-solver-comparison.json').write_text(json.dumps(rows,indent=2)+'\n')
print(out/'50k-solver-comparison.png')
