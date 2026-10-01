#!/usr/bin/env python3
"""Plot retained CCD controls; these data are not timing measurements."""
from pathlib import Path
import csv,re
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
HERE=Path(__file__).resolve().parent
pattern=re.compile(r'^ccd enabled=(\d+) frame=(\d+) y=(\S+) vy=(\S+)$',re.M)
series=[('CPU default on','ccd-safe/cpu/trial-1.stdout',1,'#d55e00','-'),('CPU shape off (failed)','cpu-diagnostic.stdout',0,'#d55e00','--'),('GPU zero-shell off (rejected)','gpu-diagnostic.stdout',0,'#0072b2','--'),('GPU broad guard off (rejected)','ccd-safe/ordinary-gpu/trial-1.stdout',0,'#56b4e9',':'),('GPU handoff off','handoff/ordinary-gpu/trial-1.stdout',0,'#0072b2','-'),('GPU default on','handoff/ordinary-gpu/trial-1.stdout',1,'#009e73','-.')]
rows=[]
for label,path,enabled,color,style in series:
 values=[(int(frame),float(y),float(vy)) for e,frame,y,vy in pattern.findall((HERE/'raw'/path).read_text()) if int(e)==enabled]
 assert values
 rows.extend(dict(series=label,frame=f,height_m=y,vertical_velocity_m_s=v,source='raw/'+path) for f,y,v in values)
fig,axes=plt.subplots(1,2,figsize=(12,5),layout='constrained')
for axis in axes:
 for label,_,_,color,style in series:
  values=[r for r in rows if r['series']==label]
  axis.plot([r['frame'] for r in values],[r['height_m'] for r in values],label=label,color=color,linestyle=style,linewidth=2,marker='o',markersize=(7 if label=='CPU shape off (failed)' else 3), markerfacecolor=('none' if label=='CPU shape off (failed)' else color))
 axis.axhline(.495,color='#555555',linestyle=':',label='Original floor bound: 0.495 m')
 axis.set_xlabel('Completed step (zero based)');axis.set_ylabel('Box center height (m)');axis.grid(alpha=.2)
axes[0].set_title('30-step CCD control');axes[0].set_xlim(-.5,29.5)
axes[1].annotate('CPU shape off and GPU zero-shell off\nboth fail at step 1: y = 0.25 m',xy=(1,.25),xytext=(.45,.32),fontsize=8,arrowprops={'arrowstyle':'->','color':'#555555'})
axes[1].set_title('Impact handoff: first three steps');axes[1].set_xlim(-.1,2.1);axes[1].set_ylim(.23,.63)
handles,labels=axes[0].get_legend_handles_labels();fig.legend(handles,labels,loc='outside lower center',ncol=2,fontsize=9)
fig.suptitle('PR01 speculative control: floor safety and rejected variants\nCPU world toggle is a stored flag; CPU shape off is the independent control',fontsize=12)
fig.savefig(HERE/'ccd-controls.png',dpi=160);fig.savefig(HERE/'ccd-controls.svg')
with (HERE/'ccd-controls.csv').open('w') as f:
 w=csv.DictWriter(f,fieldnames=list(rows[0]),lineterminator="\n");w.writeheader();w.writerows(rows)
print('Wrote chart and',len(rows),'portable rows')
