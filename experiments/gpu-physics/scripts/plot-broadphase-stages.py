#!/usr/bin/env python3
"""Compare repeated cached-broadphase diagnostics, retaining raw identities."""
import argparse
import hashlib
import json
from pathlib import Path
import statistics

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

STAGES = ['clear', 'static_setup', 'static_pairs', 'spatial_insert', 'dynamic_pairs', 'sort', 'compact']


def summarize(folder):
    manifest = json.loads((folder / 'manifest.json').read_text())
    args = manifest['arguments']
    assert args['profile_broadphase'] and args['trials'] >= 3
    assert args['workers'] == 8 and args['gpu_solver'] == 'global'
    assert args['warmup'] == 90 and args['timed'] == 240
    assert manifest['environment']['GPU_PHYSICS_PROFILE_BROADPHASE'] == '1'
    rows = json.loads((folder / 'trials.json').read_text())
    assert len(rows) == len(args['counts']) * args['trials']
    assert all(row['status'] == 'ok' for row in rows)
    results, hashes = {}, {}
    for count in args['counts']:
        group = [row for row in rows if row['count'] == count]
        assert sorted(row['trial'] for row in group) == list(range(1, args['trials'] + 1))
        stage_trials, bp_trials, completed_trials = [], [], []
        for row in group:
            path = folder / f"{count}-physics-gpu-{row['trial']}.json"
            data = json.loads(path.read_text())
            assert data['bodies'] == count + 1 and not data['sleep']
            assert data['sub_steps'] == 4 and abs(data['dt'] - 1 / 60) < 1e-7
            raw = data['raw_runs'][0]
            assert raw['physics_step'] == 330 and raw['awake_dynamic'] == count
            assert raw['live_contacts'] > 0
            profile = raw['broadphase_profile']
            assert profile['stages'] == STAGES and len(profile['samples_ms']) == 240
            assert all(len(sample) == 7 and all(0 <= v < float('inf') for v in sample)
                       for sample in profile['samples_ms'])
            means = list(map(statistics.mean, zip(*profile['samples_ms'])))
            stage_trials.append(means)
            bp_trials.append(statistics.mean(raw['broadphase_ms']))
            completed_trials.append(statistics.mean(raw['completed_step_ms']))
            assert abs(sum(means) - bp_trials[-1]) < max(.1, bp_trials[-1] * .1), 'timestamp coverage mismatch'
            hashes[path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
        results[count] = dict(stage_median_ms=list(map(statistics.median, zip(*stage_trials))),
                              stage_trial_means_ms=stage_trials,
                              broadphase_trial_means_ms=bp_trials,
                              completed_trial_means_ms=completed_trials)
    return dict(manifest=manifest, results=results, raw_sha256=hashes)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('before', type=Path)
    parser.add_argument('after', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--title', required=True)
    args = parser.parse_args()
    before, after = summarize(args.before), summarize(args.after)
    assert before['results'].keys() == after['results'].keys()
    assert before['manifest']['environment'] == after['manifest']['environment']
    counts = sorted(before['results'])
    fig, axes = plt.subplots(1, len(counts), figsize=(7 * len(counts), 5), squeeze=False, layout='constrained')
    for ax, count in zip(axes[0], counts):
        for offset, label, dataset in [(-.19, 'Baseline 49b024f5', before), (.19, 'Candidate', after)]:
            row = dataset['results'][count]
            values = row['stage_median_ms']
            ranges = list(zip(*row['stage_trial_means_ms']))
            errors = [[v - min(r) for v, r in zip(values, ranges)],
                      [max(r) - v for v, r in zip(values, ranges)]]
            ax.barh([i + offset for i in range(7)], values, height=.36, label=label, xerr=errors, capsize=3)
        ax.set_yticks(range(7), ['Clearing', 'Static setup', 'Static pairs', 'Spatial insertion',
                                'Dynamic + retained pairs', 'Radix sorting', 'Unique compaction'])
        ax.invert_yaxis()
        ax.set(title=f'{count:,} falling cubes', xlabel='Milliseconds per step (lower is better)')
        ax.grid(axis='x', alpha=.2)
        ax.legend()
    fig.suptitle(args.title)
    fig.supxlabel('Opt-in timestamps inside native cached commands • median of trial means; bars show full trial ranges\n'
                  '90 warmup + 240 timed steps • 4 substeps • sleep off • use uninstrumented runs for headline speedups', fontsize=9)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    for extension in ['png', 'svg']:
        fig.savefig(args.output.with_suffix('.' + extension), dpi=150)
    svg = args.output.with_suffix('.svg')
    svg.write_text('\n'.join(line.rstrip() for line in svg.read_text().splitlines()) + '\n')
    args.output.with_suffix('.json').write_text(json.dumps(dict(stages=STAGES, before=before, after=after), indent=2) + '\n')


if __name__ == '__main__':
    main()
