#!/usr/bin/env python3
"""Plot verified GPU phase costs and primary allocations across a completed sweep."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import runpy
import statistics

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

PHASES = ['broadphase', 'narrowphase', 'graph', 'prepare', 'solve']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--title', required=True)
    args = parser.parse_args()
    report = runpy.run_path(str(Path(__file__).with_name('plot-falling-cubes.py')))['summarize'](args.input)
    settings = report['manifest']['arguments']
    summary, hashes = [], {}
    for result in report['results']:
        if result['mode'] != 'physics-gpu':
            continue
        runs = []
        for trial in range(1, settings['trials'] + 1):
            path = args.input / f"{result['count']}-physics-gpu-{trial}.json"
            data = json.loads(path.read_text())
            assert len(data['raw_runs']) == 1
            raw = data['raw_runs'][0]
            assert raw['physics_step'] == settings['warmup'] + settings['timed']
            assert raw['awake_dynamic'] == result['count'] and raw['live_contacts'] > 0
            assert not data['sleep'] and data['sub_steps'] == 4
            assert math.isclose(data['dt'], 1 / 60, abs_tol=1e-7)
            allocation = raw['allocations']
            assert allocation['scope'] == 'primary_simulation_buffers'
            assert sum(v for k, v in allocation.items() if k.endswith('_bytes') and k != 'total_bytes') == allocation['total_bytes']
            for phase in PHASES + ['encode', 'completed_step']:
                samples = raw[phase + '_ms']
                assert len(samples) == settings['timed']
                assert all(math.isfinite(v) and v >= 0 for v in samples)
            hashes[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
            runs.append(raw)
        summary.append(dict(count=result['count'],
            mean_ms=statistics.median(statistics.mean(r['completed_step_ms']) for r in runs),
            phase_mean_ms={phase: statistics.median(statistics.mean(r[phase + '_ms']) for r in runs)
                           for phase in PHASES + ['encode']},
            allocation_bytes={key: statistics.median(r['allocations'][key] for r in runs)
                              for key in runs[0]['allocations'] if key.endswith('_bytes')},
            live_contacts=[r['live_contacts'] for r in runs]))
    assert summary, 'no complete GPU trial groups'
    fig, axes = plt.subplots(1, 2, figsize=(13, 5), layout='constrained')
    counts = [r['count'] for r in summary]
    for phase in PHASES:
        axes[0].plot(counts, [r['phase_mean_ms'][phase] for r in summary], marker='.', label=phase)
    axes[0].plot(counts, [r['mean_ms'] for r in summary], color='black', linestyle='--', label='Completed step')
    axes[0].set(title='GPU phase costs', ylabel='Milliseconds (log scale)', yscale='log')
    for keys, label in [(['total_bytes'], 'Total primary buffers'), (['contact_bytes'], 'Contacts'),
                        (['scratch_bytes', 'atom_bytes'], 'Scratch + atomics'),
                        (['body_bytes', 'shape_geometry_material_bytes', 'fixed_bytes', 'joint_bytes'], 'Bodies, scene + other')]:
        axes[1].plot(counts, [sum(r['allocation_bytes'][key] for key in keys) / 2**20 for r in summary],
                     marker='.', label=label)
    axes[1].set(title='Allocated primary simulation buffers', ylabel='MiB (log scale)', yscale='log')
    for ax in axes:
        ax.set(xscale='log', xlabel='Dynamic cubes (log scale)')
        ax.xaxis.set_major_formatter(FuncFormatter(lambda v, _: f'{v:,.0f}'))
        ax.grid(alpha=.2)
        ax.legend(fontsize=8)
    fig.suptitle(args.title)
    fig.supxlabel(f"{settings['trials']} trials • medians of trial means • sleep off • 4 substeps\n"
                  'Phase medians do not add to completed time. Memory excludes driver, staging and renderer allocations.', fontsize=9)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    for extension in ['png', 'svg']:
        fig.savefig(args.output.with_suffix('.' + extension), dpi=150)
    svg = args.output.with_suffix('.svg')
    svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines()) + '\n')
    args.output.with_suffix('.json').write_text(json.dumps(dict(results=summary, raw_sha256=hashes), indent=2) + '\n')
    print(f'Verified and plotted {len(summary)} GPU count groups.')


if __name__ == '__main__':
    main()
