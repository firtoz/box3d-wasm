#!/usr/bin/env python3
"""Compare preserved GPU binaries under one falling-cube benchmark manifest.

Alternates binary order between trials. Keeps failed runs, raw timings, phases,
hardware/power snapshots and binary hashes. Normal desktop load; no idle gate.
The full CPU/application-renderer sweep remains bench-falling-cubes.py's job.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import statistics
import subprocess
from datetime import datetime, timezone
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[1]
BENCH = runpy.run_path(str(Path(__file__).with_name('bench-falling-cubes.py')))
PHASES = ['broadphase', 'narrowphase', 'graph', 'prepare', 'solve', 'encode']


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('manifest', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--binary', action='append', required=True, metavar='LABEL=PATH')
    parser.add_argument('--counts', nargs='+', type=int, default=[100000])
    parser.add_argument('--trials', type=int, default=3)
    parser.add_argument('--timeout', type=int, default=900)
    parser.add_argument('--resume', action='store_true', help='Keep verified completed trials and archive interrupted attempts before retrying')
    args = parser.parse_args()
    if args.trials < 1 or args.timeout < 1 or any(n < 1 or n > 1000000 for n in args.counts):
        parser.error('positive trials/timeout and counts in 1..1000000 required')
    binaries = {}
    for item in args.binary:
        label, separator, value = item.partition('=')
        if not separator or not re.fullmatch(r'[a-z0-9][a-z0-9-]*', label) or label in binaries:
            parser.error('unique lowercase LABEL=PATH required')
        path = Path(value).resolve()
        binaries[label] = dict(path=str(path), sha256=digest(path))
    source = json.loads(args.manifest.read_text())
    settings = SimpleNamespace(**source['arguments'])
    assert settings.workers == 8
    assert source['environment']['GPU_PHYSICS_COMPONENT_TGS'] == '0'
    env = {k: v for k, v in os.environ.items()
           if not k.startswith(('GPU_PHYSICS_', 'GPU_SOKOL_', 'GPU_BENCH_'))}
    env.update(source['environment'])
    out = args.output.resolve()
    save = BENCH['write_json']
    manifest = dict(source=source, binaries=binaries, counts=args.counts,
         trials=args.trials, script_sha256=digest(Path(__file__)),
         helper_sha256=digest(Path(__file__).with_name('bench-falling-cubes.py')), timeout=args.timeout)
    rows = []
    if args.resume:
        original = json.loads((out / 'manifest.json').read_text())
        for key in ['source', 'binaries', 'counts', 'trials', 'timeout', 'helper_sha256']:
            if original[key] != manifest[key]:
                raise RuntimeError(f'resume mismatch: {key}')
        if list(original['binaries']) != list(binaries):
            raise RuntimeError('resume binary order changed')
        previous = json.loads((out / 'trials.json').read_text()) if (out / 'trials.json').exists() else []
        seen = set()
        for row in previous:
            key = (row['count'], row['variant'], row['trial'])
            if key in seen or key[0] not in args.counts or key[1] not in binaries or not 1 <= key[2] <= args.trials:
                raise RuntimeError(f'invalid previous trial identity: {key}')
            seen.add(key)
            if row['status'] == 'ok':
                path = out / f'{key[0]}-{key[1]}-{key[2]}.json'
                if digest(path) != row['raw_sha256']:
                    raise RuntimeError(f'completed raw data changed: {path}')
                rows.append(row)
        # Validate before changing anything. Keep original runner/manifest and every
        # interrupted log or invalid row, including interruptions without a JSON row.
        stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
        recovery = out / 'resume-history' / stamp
        recovery.mkdir(parents=True, exist_ok=False)
        save(recovery / 'previous-trials.json', previous)
        save(recovery / 'manifest.json', manifest)
        (recovery / 'runner.py').write_bytes(Path(__file__).read_bytes())
        completed = {f"{r['count']}-{r['variant']}-{r['trial']}" for r in rows}
        for count in args.counts:
            for label in binaries:
                for trial in range(1, args.trials + 1):
                    stem = f'{count}-{label}-{trial}'
                    if stem not in completed:
                        for path in out.glob(stem + '.*'):
                            if path.is_file():
                                path.rename(recovery / path.name)
        save(out / 'trials.json', rows)
    else:
        out.mkdir(parents=True, exist_ok=False)
        save(out / 'manifest.json', manifest)
        (out / 'runner.py').write_bytes(Path(__file__).read_bytes())
        (out / 'bench-falling-cubes.py').write_bytes(Path(__file__).with_name('bench-falling-cubes.py').read_bytes())
        (out / 'working-tree.patch').write_bytes(subprocess.check_output(['git', 'diff'], cwd=ROOT))
    completed = {(r['count'], r['variant'], r['trial']) for r in rows}
    for count in args.counts:
        for trial in range(1, args.trials + 1):
            order = list(binaries)
            if trial % 2 == 0:
                order.reverse()
            for label in order:
                if (count, label, trial) in completed:
                    continue
                binary = Path(binaries[label]['path'])
                if digest(binary) != binaries[label]['sha256']:
                    raise RuntimeError(f'binary changed during measurement: {binary}')
                path = out / f'{count}-{label}-{trial}.json'
                command = [str(binary), '--scene', 'falling-cubes', '--bodies', str(count),
                           '--warmup', str(settings.warmup), '--frames', str(settings.timed),
                           '--no-sleep', '--bench', str(path), '--metric-runs', '1']
                row = dict(count=count, variant=label, trial=trial, command=command,
                           before=BENCH['system']())
                print(f'{count} cubes, {label}, trial {trial}', flush=True)
                try:
                    with path.with_suffix('.log').open('w') as log:
                        subprocess.run(command, cwd=ROOT, env=env | {'GPU_BENCH_CUBES': str(count)},
                                       stdout=log, stderr=log, timeout=args.timeout, check=True)
                    data = json.loads(path.read_text())
                    row.update(BENCH['metrics'](data, 'physics-gpu', path, settings, count))
                    assert abs(data['dt'] - 1 / 60) < 1e-7 and data['sub_steps'] == 4
                    raw = data['raw_runs'][0]
                    row.update(status='ok', allocations=raw['allocations'],
                        phase_mean_ms={p: statistics.mean(raw[p + '_ms']) for p in PHASES},
                        raw_sha256=digest(path))
                except (subprocess.SubprocessError, AssertionError, ValueError, KeyError) as error:
                    row.update(status='invalid', error=str(error))
                    raise
                finally:
                    row['after'] = BENCH['system']()
                    rows.append(row)
                    save(out / 'trials.json', rows)
                print(f"  {row['mean_ms']:.3f} ms ({1000 / row['mean_ms']:.2f} steps/s)", flush=True)


if __name__ == '__main__':
    main()
