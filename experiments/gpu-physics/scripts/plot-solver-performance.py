#!/usr/bin/env python3
"""Plot completed controlled benchmark receipts; never plot a partial batch as final."""
import argparse
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

CPU = '#3569A8'
GPU = '#D76B24'


def load(path):
    data = json.loads(path.read_text())
    if data['status'] != 'complete':
        raise ValueError(f'{path}: benchmark is not complete')
    for mode in ('ordinary', 'native'):
        stats = data['paths'][mode]['statistics']
        if len(stats['pairs']) != 5:
            raise ValueError(f'{path}: need all five {mode} pairs')
    return data


def save(fig, directory, name):
    fig.savefig(directory / f'{name}.png', dpi=180, facecolor='white')
    fig.savefig(directory / f'{name}.svg', facecolor='white')
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--small', type=Path, required=True)
    parser.add_argument('--large', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    scenes = [('Falling Ragdolls', load(args.small)), ('Large Pyramid', load(args.large))]
    args.out.mkdir(parents=True, exist_ok=True)
    plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 11,
                         'axes.spines.top': False, 'axes.spines.right': False,
                         'axes.titleweight': 'bold', 'axes.grid': True,
                         'grid.alpha': .18, 'svg.fonttype': 'none'})
    legend = [Line2D([0], [0], color=CPU, marker='o', label='GPU solver: CPU-compatible ordering (1)'),
              Line2D([0], [0], color=GPU, marker='o', label='GPU solver: native ordering (0)')]
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    for row, (scene, data) in enumerate(scenes):
        for col, mode in enumerate(('ordinary', 'native')):
            ax = axes[row, col]
            pairs = data['paths'][mode]['statistics']['pairs']
            for pair in pairs:
                x = pair['pair']
                a, b = pair['cpu_compatible_ms'], pair['gpu_native_ms']
                ax.plot([x-.08, x+.08], [a, b], color='#AAA', zorder=1)
                ax.scatter(x-.08, a, color=CPU, s=44, zorder=2)
                ax.scatter(x+.08, b, color=GPU, s=44, zorder=2)
            ax.set(title=f'{scene} · {"Ordinary" if mode == "ordinary" else "Native cached"}',
                   xlabel='Matched pair (run order alternated)', ylabel='Mean latency (ms)',
                   xticks=range(1, 6), ylim=(0, 1.15 * max(max(p['cpu_compatible_ms'], p['gpu_native_ms']) for p in pairs)))
    fig.suptitle('Completed physics latency — all five measured pairs', fontsize=17, y=.99)
    fig.legend(handles=legend, loc='upper center', bbox_to_anchor=(.5, .95), ncol=2, frameon=False)
    fig.text(.5, .015, '60 warmup + 180 timed steps per fresh process. Includes GPU completion, pose synchronization and bounded counters.',
             ha='center', fontsize=10)
    fig.tight_layout(rect=(0, .04, 1, .89))
    save(fig, args.out, 'paired-mean-latency')

    fig, ax = plt.subplots(figsize=(12, 5.5))
    labels = []
    for index, (scene, data, mode) in enumerate((scene, data, mode) for scene, data in scenes for mode in ('ordinary', 'native')):
        stats = data['paths'][mode]['statistics']
        ratio = stats['geometric_mean_ratio']
        low, high = stats['ratio_ci95']
        upper = stats['ratio_upper_one_sided95']
        ax.errorbar(ratio, index, xerr=[[ratio-low], [high-ratio]], fmt='o', color=GPU,
                    markersize=8, capsize=5, linewidth=2, zorder=3)
        ax.scatter(upper, index+.13, marker='|', s=180, color=CPU, zorder=4)
        for offset, pair in zip([-.14, -.07, 0, .07, .14], stats['pairs']):
            ax.scatter(pair['ratio'], index+offset, color='#777', alpha=.55, s=22, zorder=2)
        labels.append(f'{scene} / {"Ordinary" if mode == "ordinary" else "Native cached"}')
        ax.text(1.03, index, f'{ratio:.3f}  |  {upper:.3f}',
                transform=ax.get_yaxis_transform(), va='center', fontsize=11)
    ax.axvline(1, color='#333', linestyle='--', linewidth=1.3)
    ax.set(yticks=range(4), yticklabels=labels, xlabel='Latency ratio: GPU-native / CPU-compatible (lower is faster)',
           title='Ordering comparison — ratios and uncertainty')
    ax.invert_yaxis()
    ax.margins(x=.10, y=.2)
    ax.text(1.03, 1.06, 'Ratio | upper 95%', transform=ax.transAxes, fontsize=11, weight='bold')
    fig.legend(handles=[Line2D([0], [0], color=GPU, marker='o', label='Geometric mean + two-sided 95% interval'),
                       Line2D([0], [0], color=CPU, marker='|', linestyle='None', markersize=12,
                              label='One-sided 95% upper bound'),
                       Line2D([0], [0], color='#777', marker='o', linestyle='None', label='Individual paired ratio')],
              loc='lower center', bbox_to_anchor=(.5, .10), ncol=2, frameon=False)
    fig.text(.5, .015, 'Five paired log-ratios per cell, Student-t (df=4). Mean-speed screen requires the one-sided upper bound below 1.',
             ha='center', fontsize=10)
    fig.subplots_adjust(left=.29, right=.75, top=.86, bottom=.33)
    save(fig, args.out, 'latency-ratios')

    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    for row, (scene, data) in enumerate(scenes):
        for col, mode in enumerate(('ordinary', 'native')):
            ax = axes[row, col]
            for pair in data['paths'][mode]['statistics']['pairs']:
                x = pair['pair']
                for dx, prefix, color in [(-.08, 'cpu', CPU), (.08, 'gpu', GPU)]:
                    ax.plot([x+dx, x+dx], [pair[f'{prefix}_p95_ms'], pair[f'{prefix}_max_ms']],
                            color=color, alpha=.65)
                    ax.scatter(x+dx, pair[f'{prefix}_p95_ms'], color=color, marker='o', s=40)
                    ax.scatter(x+dx, pair[f'{prefix}_max_ms'], color=color, marker='^', s=45)
            ax.set(title=f'{scene} · {"Ordinary" if mode == "ordinary" else "Native cached"}',
                   xlabel='Matched pair', ylabel='Latency (ms)', xticks=range(1, 6),
                   ylim=(0, 1.12 * max(max(p['cpu_max_ms'], p['gpu_max_ms'])
                                      for p in data['paths'][mode]['statistics']['pairs'])))
    fig.suptitle('Tail latency — p95 (circles) and maximum (triangles)', fontsize=17, y=.99)
    fig.legend(handles=legend, loc='upper center', bbox_to_anchor=(.5, .95), ncol=2, frameon=False)
    fig.text(.5, .015, 'Every measured run is shown. A mean-speed improvement does not establish uniform tail improvement or physical correctness.',
             ha='center', fontsize=10)
    fig.tight_layout(rect=(0, .04, 1, .89))
    save(fig, args.out, 'tail-latency')
    (args.out/'index.html').write_text('''<!doctype html><meta charset="utf-8"><title>GPU solver performance</title>
<style>body{font:16px system-ui;max-width:1200px;margin:32px auto;padding:0 20px;color:#222}img{width:100%}p{line-height:1.5}</style>
<h1>GPU solver performance</h1><p>Controlled completed-step measurements. These compare two GPU constraint-ordering modes, not GPU versus the CPU engine. Mode 0 also includes the paired-normal solver policy; these results do not isolate sorting cost. Physics qualification and the default decision are separate.</p>
''' + ''.join(f'<h2>{title}</h2><img src="{name}.png"><p><a href="{name}.svg">Download SVG</a></p>' for name,title in [
        ('paired-mean-latency','All five pairs'),('latency-ratios','Ratios and uncertainty'),('tail-latency','Tail latency')]))
    print(args.out.resolve()/'index.html')


if __name__ == '__main__':
    main()
