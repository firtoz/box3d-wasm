#!/usr/bin/env python3
"""Plot validated solver-cost pilots and the synthetic dispatch-boundary probe."""
import argparse
import json
from pathlib import Path
import statistics


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--title', required=True)
    args = parser.parse_args()
    pilots = json.loads((args.input / 'pilots-summary.json').read_text())
    probe = json.loads((args.input / 'boundary-probe.json').read_text())
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    plt.rcParams.update({'font.size': 10, 'axes.spines.top': False, 'axes.spines.right': False})
    fig, axes = plt.subplots(2, 2, figsize=(13, 8))
    labels = {
        'impulse': ('Impulse-only contact writes', 'control: commit 0bbf5f63'),
        'velocity': ('Also narrow body writes — rejected', 'control: impulse-only writes'),
        'specialize': ('Specialize broad-color shaders — rejected', 'control: impulse-only writes'),
        'all-specialize': ('Specialize all color shaders — rejected', 'control: impulse-only writes'),
    }
    for ax, (key, (label, control)) in zip(axes.flat, labels.items()):
        data = pilots[key]
        for offset, metric, color, name in [(-.18, 'completed_ms', '#2374ab', 'Completed physics step'),
                                          (.18, 'solver_ms', '#dc7930', 'GPU solver phase')]:
            gain = [100 * (1 - row[metric]['after'] / row[metric]['before']) for row in data]
            bars = ax.bar([i + offset for i in range(len(data))], gain, width=.34, label=name, color=color)
            ax.bar_label(bars, labels=[f'{v:+.1f}%' for v in gain], padding=3, fontsize=9)
        ax.set_xticks(range(len(data)), [f"{r['count']:,}" for r in data])
        ax.set_ylim(-24, 25)
        ax.axhline(0, color='#777777', linewidth=.7)
        ax.set_title(label + '\n' + control, fontsize=11)
        ax.set_ylabel('Time reduction (%) — higher is better')
        ax.set_xlabel('Falling cubes')
        ax.grid(axis='y', alpha=.15)
    axes[0, 0].legend(loc='upper left', fontsize=8)
    fig.suptitle(args.title + '\nDiagnostic pilots: median of 3 trial means, 90 warmup + 240 timed steps', fontsize=13)
    fig.text(.5, .008, 'Each panel has its own control. These are experiments, not final qualification. GPU phase timings overlap host work.', ha='center', fontsize=9)
    fig.tight_layout(rect=(0, .025, 1, .94))
    save(fig, args.output, '-pilots')

    fig, ax = plt.subplots(figsize=(10, 5.6))
    for grouped, label, color, offset in [(False, '260 separate dispatches', '#2374ab', -.18),
                                         (True, '13 grouped dispatches + barriers', '#dc7930', .18)]:
        medians, low, high = [], [], []
        for width in [64, 256, 1024]:
            values = [r['ms'] for r in probe['raw'] if r['width'] == width and r['grouped'] == grouped and r['trial'] > 0]
            assert len(values) == 7
            median = statistics.median(values)
            medians.append(median); low.append(median - min(values)); high.append(max(values) - median)
        bars = ax.bar([i + offset for i in range(3)], medians, width=.34, color=color, label=label,
                      yerr=[low, high], capsize=4)
        ax.bar_label(bars, labels=[f'{v:.2f}' for v in medians], padding=4)
    ax.set_xticks(range(3), ['64', '256', '1,024'])
    ax.set_xlabel('Synthetic contacts per color visit')
    ax.set_ylabel('GPU interval (ms) — lower is better')
    ax.set_title(args.title + '\nDispatch boundaries versus workgroup barriers — synthetic probe', fontsize=13)
    ax.legend(loc='upper left')
    ax.grid(axis='y', alpha=.15)
    fig.text(.5, .02, 'Identical prepared state restored; 20 color visits × 13 waves; complete outputs match byte-for-byte.\n'
             'Median + full range of 7 alternating pairs (warmup retained separately). Same contacts reused each color.\n'
             'Within-width scheduling comparison only; absolute times are not a falling-scene cost breakdown or scaling curve.',
             ha='center', fontsize=9)
    fig.tight_layout(rect=(0, .13, 1, 1))
    save(fig, args.output, '-boundaries')


def save(fig, prefix, suffix):
    prefix.parent.mkdir(parents=True, exist_ok=True)
    for extension in ['png', 'svg']:
        path = Path(str(prefix) + suffix + '.' + extension)
        fig.savefig(path, dpi=160, bbox_inches='tight', pad_inches=.15)
        if extension == 'svg':
            path.write_text('\n'.join(line.rstrip() for line in path.read_text().splitlines()) + '\n')


if __name__ == '__main__':
    main()
