#!/usr/bin/env python3
"""Plot matched physics and renderer throughput/latency from verified raw trials."""
import argparse
import hashlib
import json
from pathlib import Path
import statistics

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


def load(path):
    return json.loads(path.read_text())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('physics', type=Path)
    parser.add_argument('renderers', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--title', required=True)
    args = parser.parse_args()
    source=load(args.physics / 'manifest.json')['source']
    other=load(args.renderers / 'manifest.json')['source']
    assert source['environment']==other['environment']
    assert 'GPU_PHYSICS_PROFILE_BROADPHASE' not in source['environment']
    assert source['arguments']['workers']==8 and source['arguments']['gpu_solver']=='global'
    physics, renderers = load(args.physics / 'trials.json'), load(args.renderers / 'trials.json')
    assert len(physics) == 30 and len(renderers) == 24
    assert all(row['status'] == 'ok' for row in physics + renderers)
    summary, hashes = [], {}
    paths = [('physics', 'Completed physics', 'Steps/s'),
             ('direct-gpu', 'Direct instanced renderer', 'FPS'),
             ('sokol-gpu', 'Sokol sample app', 'FPS')]
    fig, axes = plt.subplots(2, 3, figsize=(15, 8), layout='constrained')
    for column, (mode, title, unit) in enumerate(paths):
        for x, count in enumerate([100000, 200000]):
            for variant, offset, color in [('before', -.18, '#426c9c'), ('after', .18, '#d47f35')]:
                if mode == 'physics':
                    label = {'before': 'before-49b024f5', 'after': 'after-hash-growth'}[variant]
                    rows = [r for r in physics if r['count'] == count and r['variant'] == label]
                else:
                    rows = [r for r in renderers if r['count'] == count and r['variant'] == variant and r['mode'] == mode]
                assert sorted(r['trial'] for r in rows) == [1, 2, 3]
                for row in rows:
                    raw_path = (args.physics / f"{count}-{row['variant']}-{row['trial']}.json" if mode == 'physics'
                                else args.renderers / row['directory'] / f'{count}-{mode}-1.json')
                    digest = hashlib.sha256(raw_path.read_bytes()).hexdigest()
                    assert digest == row['raw_sha256'], raw_path
                    hashes[str(raw_path)] = digest
                means = [r['mean_ms'] for r in rows]
                mean = statistics.median(means)
                rate = 1000 / mean
                p50, p95 = (statistics.median(r[key] for r in rows) for key in ['p50_ms', 'p95_ms'])
                axes[0, column].bar(x + offset, rate, width=.34, color=color,
                                    label=variant.title() if x == 0 else None)
                axes[0, column].errorbar(x + offset, rate,
                    yerr=[[rate - 1000 / max(means)], [1000 / min(means) - rate]],
                    fmt='none', color='black', capsize=3)
                axes[0, column].annotate(f'{rate:.2f}', (x + offset, 1000 / min(means)),
                                         xytext=(0, 4), textcoords='offset points', ha='center', fontsize=9)
                axes[1, column].bar(x + offset, p50, width=.34, color=color,
                                    label=variant.title() + ' p50' if x == 0 else None)
                axes[1, column].scatter(x + offset, p95, color=color, marker='D', edgecolors='black',
                                        zorder=4, label=variant.title() + ' p95' if x == 0 else None)
                summary.append(dict(mode=mode, count=count, variant=variant, rate=rate,
                                    mean_ms=mean, trial_means_ms=means, p50_ms=p50, p95_ms=p95))
        axes[0, column].set(title=title, ylabel=unit + ' (higher is better)')
        axes[1, column].set(ylabel='Milliseconds (lower is better)', xlabel='Dynamic cubes')
        for row in [0, 1]:
            ax = axes[row, column]
            ax.set_xticks([0, 1], ['100,000', '200,000'])
            ax.set_ylim(0, ax.get_ylim()[1] * 1.2)
            ax.grid(axis='y', alpha=.2)
            ax.set_axisbelow(True)
            ax.legend(fontsize=8, ncol=2)
    fig.suptitle(args.title)
    fig.supxlabel('3 alternating before/after trials • native caches on • normal desktop load • no profiling timestamps\n'
                  '90 warmup + 240 timed steps • 4 substeps • sleep off • rates: reciprocal of median trial mean; error bars: full trial range\n'
                  'Latency: medians of trial p50/p95; not pooled percentiles. Compare builds within each renderer.', fontsize=9)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    for ext in ['png', 'svg']:
        fig.savefig(args.output.with_suffix('.' + ext), dpi=150)
    svg = args.output.with_suffix('.svg')
    svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines()) + '\n')
    args.output.with_suffix('.json').write_text(json.dumps(dict(results=summary, raw_sha256=hashes), indent=2) + '\n')


if __name__ == '__main__':
    main()
