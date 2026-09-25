#!/usr/bin/env python3
"""Validate a complete startup grid and plot per-renderer medians and trial ranges."""
import argparse
import hashlib
import itertools
import json
import math
from pathlib import Path
import statistics


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def summarize(folder):
    manifest = json.loads((folder / 'manifest.json').read_text())
    settings = manifest['settings']
    rows = json.loads((folder / 'trials.json').read_text())
    assert settings['trials'] >= 3, 'At least three trials are required'
    assert set(settings['modes']) == {'direct', 'sokol'}, 'Both renderers are required'
    expected = set(itertools.product(settings['counts'], settings['modes'],
                                    ['before', 'after'], ['cold', 'warm'],
                                    range(1, settings['trials'] + 1)))
    seen, groups, identities, dimensions = set(), {}, {}, {}
    for row in rows:
        if row['status'] != 'ok':
            assert row['status'] != 'running', 'Benchmark still running'
            continue  # Retain failed/interrupted attempts in the evidence, not the statistics.
        key = (row['count'], row['mode'], row['variant'], row['cache'], row['trial'])
        assert key in expected and key not in seen, ('Unexpected or duplicate trial', key)
        seen.add(key)
        raw, log = folder / row['raw'], folder / row['log']
        assert sha256(raw) == row['raw_sha256'], raw
        assert sha256(log) == row['log_sha256'], log
        identities[raw.name], identities[log.name] = sha256(raw), sha256(log)
        data = json.loads(raw.read_text())
        assert data == row['result'] and data['dynamic_cubes'] == row['count']
        fields = ['scene_ms', 'device_ms', 'gpu_prepare_ms', 'first_frame_ms']
        for field in fields:
            assert isinstance(data[field], (int, float)) and math.isfinite(data[field]) and data[field] >= 0
        assert data['first_frame_ms'] >= data['scene_ms'] + data['gpu_prepare_ms']
        loading = data['first_loading_frame_ms']
        if loading is not None:
            assert 0 <= loading <= data['first_frame_ms'] and data['loading_frames'] > 0
        if row['variant'] == 'after':
            assert loading is not None, ('Candidate never presented loading', key)
        loaded = row['pipeline_cache_loaded_bytes']
        assert loaded and (all(v == 0 for v in loaded) if row['cache'] == 'cold' else any(v > 0 for v in loaded))
        size = row['framebuffer']
        assert size == dimensions.setdefault(row['mode'], size), 'Framebuffer mismatch'
        groups.setdefault(key[:-1], []).append(data)
    assert seen == expected, f'Incomplete grid: {len(expected - seen)} missing trials'
    summaries = []
    for key, trials in sorted(groups.items()):
        metrics = {}
        for field in fields + ['first_loading_frame_ms', 'loading_frames']:
            values = [r[field] for r in trials if r[field] is not None]
            metrics[field] = None if not values else dict(median=statistics.median(values),
                minimum=min(values), maximum=max(values), trials=values)
        summaries.append(dict(count=key[0], mode=key[1], variant=key[2], cache=key[3], metrics=metrics))
    annotations = folder / 'capture-pause-note.json'
    return dict(manifest=manifest, framebuffer=dimensions, groups=summaries, raw_sha256=identities,
                annotations=json.loads(annotations.read_text()) if annotations.exists() else None,
                excluded_attempts=[r for r in rows if r['status'] != 'ok'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    parser.add_argument('output', type=Path, help='Output prefix for PNG/SVG and summary JSON')
    parser.add_argument('--title', required=True, help='Exact hardware and date')
    parser.add_argument('--validate-only', action='store_true')
    args = parser.parse_args()
    data = summarize(args.folder)
    if args.validate_only:
        print(f"Validated {len(data['groups'])} groups; complete raw/log hashes and cache checks")
        return
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    counts = sorted(data['manifest']['settings']['counts'])
    trials = data['manifest']['settings']['trials']
    lookup = {(r['count'], r['mode'], r['variant'], r['cache']): r['metrics'] for r in data['groups']}
    colors = {'before': '#cf613a', 'after': '#137ca3'}
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def series(ax, mode, variant, cache, field, scale=1):
        rows = [lookup[(n, mode, variant, cache)][field] for n in counts]
        if all(r is None for r in rows):
            return
        assert all(r is not None for r in rows), 'Inconsistent loading frame availability'
        med = [r['median'] / scale for r in rows]
        error = [[(r['median'] - r['minimum']) / scale for r in rows],
                 [(r['maximum'] - r['median']) / scale for r in rows]]
        label = f"{'Baseline' if variant == 'before' else 'Candidate'} / {cache}"
        ax.errorbar(counts, med, yerr=error, marker='o', capsize=3,
                    color=colors[variant], linestyle='-' if cache == 'cold' else '--', label=label)

    def finish(fig, suffix, note):
        fig.suptitle(args.title)
        fig.supxlabel(note, fontsize=9)
        prefix = args.output.with_name(args.output.name + '-' + suffix)
        for ext in ['png', 'svg']:
            fig.savefig(prefix.with_suffix('.' + ext), dpi=150)
        plt.close(fig)

    fig, axes = plt.subplots(2, 2, figsize=(13, 9), layout='constrained')
    for column, mode in enumerate(['direct', 'sokol']):
        ax = axes[0, column]
        for variant, cache in itertools.product(['before', 'after'], ['cold', 'warm']):
            series(ax, mode, variant, cache, 'scene_ms')
        ax.set(title=f'{mode.title()}: scene creation', ylabel='Milliseconds (log scale)', xscale='log', yscale='log')
        ax.legend(fontsize=8)
        ax = axes[1, column]
        for cache in ['cold', 'warm']:
            speedups = [lookup[(n, mode, 'before', cache)]['scene_ms']['median'] /
                        lookup[(n, mode, 'after', cache)]['scene_ms']['median'] for n in counts]
            ax.plot(counts, speedups, 'o-', label=cache)
        ax.axhline(10, color='gray', linestyle=':', label='10× target')
        ax.set(title='Scene creation speedup', ylabel='Baseline / candidate (log scale)', xscale='log', yscale='log')
        ax.legend(fontsize=8)
    for ax in axes.flat:
        ax.set_xlabel('Dynamic cubes'); ax.grid(alpha=.2)
        ax.set_xticks(counts, [f'{n / 1000:g}k' for n in counts])
    finish(fig, 'scene', f'Median of {trials} trials; error bars show full trial ranges. Speedup uses ratio of medians.\nScene creation excludes device initialization, GPU preparation and rendering.')

    fig, axes = plt.subplots(4, 2, figsize=(13, 15), layout='constrained')
    metrics = [('device_ms', 'Device initialization'), ('gpu_prepare_ms', 'GPU preparation'),
               ('first_loading_frame_ms', 'First visible loading frame'), ('first_frame_ms', 'First scene frame')]
    for column, mode in enumerate(['direct', 'sokol']):
        for row, (field, label) in enumerate(metrics):
            ax = axes[row, column]
            for variant, cache in itertools.product(['before', 'after'], ['cold', 'warm']):
                series(ax, mode, variant, cache, field, scale=1000)
            ax.set(title=f'{mode.title()}: {label}', xlabel='Dynamic cubes', ylabel='Seconds (log scale)', xscale='log', yscale='log')
            if field == 'device_ms' or (mode == 'direct' and field == 'first_loading_frame_ms'):
                ax.set(yscale='linear', ylabel='Seconds', ylim=(0, None))
            ax.set_xticks(counts, [f'{n / 1000:g}k' for n in counts]); ax.grid(alpha=.2); ax.legend(fontsize=8)
            if mode == 'direct' and field == 'first_loading_frame_ms':
                ax.text(.03, .96, 'Baseline has no loading frame', transform=ax.transAxes, va='top', fontsize=9)
    finish(fig, 'latency', f'Median and full range of {trials} trials. Fresh persistent caches vs immediate reuse; driver internal caches not controlled.\nDirect starts before window creation; Sokol starts in OnInit. Compare before/after within each renderer.')
    args.output.with_suffix('.json').write_text(json.dumps(data, indent=2) + '\n')


if __name__ == '__main__':
    main()
