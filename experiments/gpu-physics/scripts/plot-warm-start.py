#!/usr/bin/env python3
"""Plot the paired warm-start timing receipt without claiming a speedup."""
import json
from pathlib import Path
import statistics
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

root = Path(__file__).resolve().parents[1]
data = json.loads((root / 'benchmarks/2026-09-29-warm-start-timings.json').read_text())
labels, baseline, candidate, changes = [], [], [], []
for scene in ['sphere-ground', 'spherical-joint']:
    for substeps in [1, 4]:
        values = {}
        for arm in ['baseline', 'candidate']:
            values[arm] = [next(row['mean_ms'] for row in run['rows']
                if row['scene'] == scene and row['substeps'] == substeps)
                for run in data['runs'] if run['arm'] == arm and not run['prewarm']]
        labels.append(f'{scene}\n{substeps} substep' + ('s' if substeps > 1 else ''))
        baseline.append(statistics.mean(values['baseline']))
        candidate.append(statistics.mean(values['candidate']))
        changes.append([100 * (c / b - 1) for b, c in zip(values['baseline'], values['candidate'])])
fig, (ax, delta) = plt.subplots(1, 2, figsize=(11, 4.8), gridspec_kw={'width_ratios': [1.5, 1]})
x = list(range(len(labels)))
ax.bar([i-.18 for i in x], baseline, .35, color='#94a3b8', label='Before')
ax.bar([i+.18 for i in x], candidate, .35, color='#2563eb', label='After')
for i, (b, c) in enumerate(zip(baseline, candidate)):
    ax.text(i-.18, b+.035, f'{b:.3f}', ha='center', fontsize=9)
    ax.text(i+.18, c+.035, f'{c:.3f}', ha='center', fontsize=9)
ax.set_xticks(x, labels, fontsize=9)
ax.set_ylabel('Completed step + pose read (ms)')
ax.set_ylim(0, max(baseline+candidate)*1.2)
ax.legend(frameon=False)
ax.spines[['top','right']].set_visible(False)
for i, points in enumerate(changes):
    delta.scatter(points, [i]*len(points), color='#2563eb', s=28, alpha=.65)
delta.axvline(0, color='#64748b', linewidth=1)
delta.set_yticks(x, labels, fontsize=9)
delta.invert_yaxis()
delta.set_xlabel('Paired change (%)\nNegative = faster; positive = slower')
delta.spines[['top','right']].set_visible(False)
fig.suptitle('Warm-start controls: default-enabled timing check', fontsize=15)
fig.text(.5, .015, 'RTX 4070 SUPER / Vulkan · 5 paired processes · 60 warmup + 180 timed steps per fixture\nDots show individual paired runs; small differences are not evidence of a speedup.', ha='center', fontsize=9)
fig.tight_layout(rect=(0,.10,1,.93))
out=root/'artifacts/warm-start/charts';out.mkdir(parents=True,exist_ok=True)
fig.savefig(out/'timings.png',dpi=160)
fig.savefig(out/'timings.svg')
for label,b,c in zip(labels,baseline,candidate):
    print(label.replace('\n', ' / '), f'{b:.6f} -> {c:.6f} ms ({100*(c/b-1):+.2f}%)')
