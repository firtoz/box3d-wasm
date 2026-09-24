#!/usr/bin/env python3
"""Compare existing solver schedules with identical falling-cube inputs.

Read hardware/environment/binary provenance from a benchmark manifest. Alternate
variant order between trials. No load gate; retain raw timings and snapshots.
This is diagnostic evidence, not a substitute for the full CPU/renderer sweep.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import runpy
import statistics
import subprocess
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[1]
BENCH = runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))
VARIANTS = {
    'component': {'GPU_PHYSICS_GRAPH_BATCHED': '0'},
    'global-colors': {'GPU_PHYSICS_COMPONENT_TGS': '0', 'GPU_PHYSICS_COLOR_PREFIX': '20',
                      'GPU_PHYSICS_GRAPH_BATCHED': '0'},
    'batched-global': {'GPU_PHYSICS_COMPONENT_TGS': '0', 'GPU_PHYSICS_COLOR_PREFIX': '20',
                       'GPU_PHYSICS_GRAPH_BATCHED': '1'},
}
PHASES = ['broadphase', 'narrowphase', 'graph', 'prepare', 'solve']


def save(path, data):
    BENCH['write_json'](path, data)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('manifest', type=Path)
    p.add_argument('output', type=Path)
    p.add_argument('--counts', type=int, nargs='+', default=[5000, 15000])
    p.add_argument('--trials', type=int, default=3)
    p.add_argument('--variants', nargs='+', choices=list(VARIANTS), default=['component', 'global-colors'])
    a = p.parse_args()
    if a.trials < 1 or any(n < 1 for n in a.counts):
        p.error('positive counts and trials required')
    source = json.loads(a.manifest.read_text())
    variants = {name: VARIANTS[name] for name in a.variants}
    binary = Path(source['binaries']['gpu']['path'])
    digest = hashlib.sha256(binary.read_bytes()).hexdigest()
    if digest != source['binaries']['gpu']['sha256']:
        p.error('binary changed since source manifest')
    if source['environment'].get('GPU_PHYSICS_COMPONENT_TGS') != '1':
        p.error('source manifest must enable component TGS')
    out = a.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(('GPU_PHYSICS_', 'GPU_SOKOL_', 'GPU_BENCH_'))}
    env.update(source['environment'])
    args = SimpleNamespace(**source['arguments'])
    save(out / 'manifest.json', dict(source=source, variants=variants,
         counts=a.counts, trials=a.trials, binary_sha256=digest,
         script_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()))
    rows = []
    for count in a.counts:
        for trial in range(1, a.trials + 1):
            order = list(variants)
            if trial % 2 == 0:
                order.reverse()
            for variant in order:
                path = out / f'{count}-{variant}-{trial}.json'
                command = [str(binary), '--scene', 'falling-cubes', '--bodies', str(count),
                           '--warmup', str(args.warmup), '--frames', str(args.timed),
                           '--no-sleep', '--bench', str(path), '--metric-runs', '1']
                row = dict(count=count, variant=variant, trial=trial, command=command,
                           before=BENCH['system']())
                with path.with_suffix('.log').open('w') as log:
                    result = subprocess.run(command, cwd=ROOT,
                        env=env | VARIANTS[variant] | {'GPU_BENCH_CUBES': str(count)},
                        stdout=log, stderr=log, timeout=600)
                row.update(after=BENCH['system'](), returncode=result.returncode)
                if result.returncode:
                    rows.append(row)
                    save(out / 'trials.json', rows)
                    raise RuntimeError(f'Invalid trial: {path.with_suffix(".log")}')
                data = json.loads(path.read_text())
                row.update(BENCH['metrics'](data, 'physics-gpu', path, args, count))
                assert data['dt'] == 1 / 60 or abs(data['dt'] - 1 / 60) < 1e-7
                assert data['sub_steps'] == 4
                raw = data['raw_runs'][0]
                row['phase_mean_ms'] = {name: statistics.mean(raw[name + '_ms']) for name in PHASES}
                row['raw_sha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
                rows.append(row)
                save(out / 'trials.json', rows)
                print(count, variant, trial, f"{row['mean_ms']:.3f} ms", flush=True)
    summary = []
    for count in a.counts:
        for variant in variants:
            group = [r for r in rows if r['count'] == count and r['variant'] == variant]
            means = [r['mean_ms'] for r in group]
            summary.append(dict(count=count, variant=variant,
                mean_ms=statistics.median(means), min_ms=min(means), max_ms=max(means),
                p50_ms=statistics.median(r['p50_ms'] for r in group),
                p95_ms=statistics.median(r['p95_ms'] for r in group),
                phase_mean_ms={name: statistics.median(r['phase_mean_ms'][name] for r in group)
                               for name in PHASES}))
    save(out / 'summary.json', summary)


if __name__ == '__main__':
    main()
