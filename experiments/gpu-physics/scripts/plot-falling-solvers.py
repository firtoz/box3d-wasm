#!/usr/bin/env python3
"""Plot raw-verified solver-schedule diagnostics (requires matplotlib)."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import statistics

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

PHASES = ['broadphase', 'narrowphase', 'graph', 'prepare', 'solve']
COLORS = ['#56a7b8', '#83ba68', '#d89442', '#a69acf', '#c55d60']


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('input', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--title', default='GPU solver scheduling investigation')
    p.add_argument('--note', default='Experimental timings; solver equivalence unresolved.')
    a = p.parse_args()
    manifest = json.loads((a.input / 'manifest.json').read_text())
    rows = json.loads((a.input / 'trials.json').read_text())
    identities = [(r['count'], r['variant'], r['trial']) for r in rows]
    assert len(set(identities)) == len(identities), 'duplicate trial'
    for row in rows:
        path = a.input / f"{row['count']}-{row['variant']}-{row['trial']}.json"
        assert row['returncode'] == 0, 'invalid trial'
        assert hashlib.sha256(path.read_bytes()).hexdigest() == row['raw_sha256']
        data = json.loads(path.read_text())
        raw = data['raw_runs'][0]
        if 'allocations' in raw:
            allocation = raw['allocations']
            assert allocation['scope'] == 'primary_simulation_buffers'
            assert sum(v for k,v in allocation.items() if k.endswith('_bytes') and k != 'total_bytes') == allocation['total_bytes']
            row['allocations'] = allocation
        assert data['bodies'] == row['count'] + 1 and not data['sleep']
        assert data['sub_steps'] == 4 and raw['live_contacts'] > 0
        for key in ['p50_ms', 'p95_ms']:
            assert row[key] == data['completed_step'][key], 'cached percentile mismatch'
        settings = manifest['source']['arguments']
        assert raw['physics_step'] == settings['warmup'] + settings['timed']
        for name in ['completed_step', *PHASES]:
            values = raw[name + '_ms']
            assert len(values) == settings['timed']
            assert all(math.isfinite(v) and v >= 0 for v in values)
            expected = row['mean_ms'] if name == 'completed_step' else row['phase_mean_ms'][name]
            assert math.isclose(statistics.mean(values), expected, abs_tol=1e-9)
    memory = all('allocations' in r for r in rows)
    fig, axes = plt.subplots(1, 3 if memory else 2, figsize=(17 if memory else 12, 5), layout='constrained')
    labels, summary = [], []
    for count in manifest['counts']:
        for variant in manifest['variants']:
            group = [r for r in rows if r['count'] == count and r['variant'] == variant]
            assert len(group) == manifest['trials'], 'incomplete group'
            x = len(labels)
            labels.append(f'{count:,}\n{variant}' if len(manifest['variants']) > 1 else f'{count:,}')
            means = [r['mean_ms'] for r in group]
            mean = statistics.median(means)
            axes[0].bar(x, mean, color={'component': '#426c9c', 'global-colors': '#55a28b',
                                      'batched-global': '#9b70b8', 'before': '#426c9c', 'after': '#55a28b'}[variant])
            axes[0].errorbar(x, mean, yerr=[[mean - min(means)], [max(means) - mean]],
                            fmt='none', color='black', capsize=4)
            axes[0].text(x, max(max(means), statistics.median(r['p95_ms'] for r in group)) + 1.2, f'{mean:.1f}', ha='center', fontsize=9)
            axes[0].scatter(x, statistics.median(r['p50_ms'] for r in group),
                            marker='D', s=22, color='black', zorder=4,
                            label='Median trial p50' if x == 0 else None)
            axes[0].scatter(x, statistics.median(r['p95_ms'] for r in group),
                            marker='o', s=30, facecolors='none', edgecolors='black', zorder=4,
                            label='Median trial p95' if x == 0 else None)
            bottom = 0
            phases = {}
            for name, color in zip(PHASES, COLORS):
                value = statistics.median(r['phase_mean_ms'][name] for r in group)
                phases[name] = value
                axes[1].bar(x, value, bottom=bottom, color=color, label=name if x == 0 else None)
                bottom += value
            summary.append(dict(count=count, variant=variant, mean_ms=mean,
                                min_ms=min(means), max_ms=max(means), phase_mean_ms=phases))
            summary[-1].update(p50_ms=statistics.median(r['p50_ms'] for r in group),
                               p95_ms=statistics.median(r['p95_ms'] for r in group))
            if memory:
                allocation = {k: statistics.median(r['allocations'][k] for r in group)
                              for k in group[0]['allocations'] if k.endswith('_bytes')}
                summary[-1]['primary_buffer_bytes'] = allocation
                bottom = 0
                for keys, label, color in [
                    (['body_bytes','shape_geometry_material_bytes'], 'Bodies + scene', '#56a7b8'),
                    (['contact_bytes'], 'Contacts', '#d89442'),
                    (['scratch_bytes','atom_bytes'], 'Scratch + atomics', '#a69acf'),
                    (['fixed_bytes','joint_bytes'], 'Other primary buffers', '#83ba68')]:
                    value = sum(allocation[k] for k in keys) / 2**20
                    axes[2].bar(x, value, bottom=bottom, color=color, label=label if x == 0 else None)
                    bottom += value
                axes[2].text(x, bottom + 8, f'{bottom:.0f}', ha='center', fontsize=9)
    axes[0].set_title('Completed step • trial mean medians and ranges')
    axes[0].legend(loc='upper left', fontsize=8)
    axes[1].set_title('GPU phases • median of per-trial means')
    axes[1].legend(loc='upper left', fontsize=8)
    for ax in axes:
        ax.set_xticks(range(len(labels)), labels, fontsize=9)
        ax.set_ylabel('Milliseconds (lower is better)')
        ax.grid(axis='y', alpha=.15)
        ax.set_axisbelow(True)
    if memory:
        axes[2].set_title('Primary simulation buffer allocation')
        axes[2].set_ylabel('MiB')
        axes[2].legend(loc='upper left', fontsize=8)
        axes[2].set_ylim(0, axes[2].get_ylim()[1] * 1.25)
    if memory:
        for rate in [60, 30, 10]:
            ms = 1000 / rate
            if ms < axes[0].get_ylim()[1]:
                axes[0].axhline(ms, color='#666666', linestyle=':', linewidth=.8)
                axes[0].text(.98, ms, f'{rate} steps/s', transform=axes[0].get_yaxis_transform(),
                             ha='right', va='bottom', fontsize=8)
    fig.suptitle(a.title)
    fig.supxlabel(f"{manifest['trials']} {'trial' if manifest['trials'] == 1 else 'trials'} • sleep off • dt 1/60; 4 substeps • "
                  f"{settings['warmup']} warmup + {settings['timed']} timed steps\n"
                  + a.note + ' Phase medians are not an exact total breakdown.', fontsize=9)
    a.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(a.output.with_suffix('.png'), dpi=150)
    fig.savefig(a.output.with_suffix('.svg'))
    svg = a.output.with_suffix('.svg')
    svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines()) + '\n')
    a.output.with_suffix('.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
