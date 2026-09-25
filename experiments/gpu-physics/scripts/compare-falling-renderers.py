#!/usr/bin/env python3
"""Alternate preserved GPU builds in both falling-cube application renderers."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import runpy
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('baseline_manifest', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--gpu-binary', type=Path, required=True)
    parser.add_argument('--sokol-gpu-binary', type=Path, required=True)
    parser.add_argument('--counts', type=int, nargs='+', default=[100000, 200000])
    parser.add_argument('--trials', type=int, default=3)
    parser.add_argument('--resume', action='store_true', help='Preserve completed trials and archive incomplete attempts before retrying')
    args = parser.parse_args()
    assert args.trials > 0 and all(0 < n <= 1000000 for n in args.counts)
    source = json.loads(args.baseline_manifest.read_text())
    assert source['arguments']['workers'] == 8 and source['arguments']['gpu_solver'] == 'global'
    assert source['arguments']['warmup'] == 90 and source['arguments']['timed'] == 240
    assert 'GPU_PHYSICS_PROFILE_BROADPHASE' not in source['environment']
    binaries = {'before': {k: source['binaries'][k] for k in ['gpu', 'sokol_gpu']},
                'after': {k: dict(path=str(p.resolve()), sha256=digest(p)) for k, p in
                          [('gpu', args.gpu_binary), ('sokol_gpu', args.sokol_gpu_binary)]}}
    for variant in binaries.values():
        for identity in variant.values():
            assert digest(Path(identity['path'])) == identity['sha256']
    out = args.output.resolve()
    manifest = dict(source=source, binaries=binaries, counts=args.counts,
                    trials=args.trials, runner_sha256=digest(Path(__file__)))
    helper = runpy.run_path(str(ROOT / 'scripts/bench-falling-cubes.py'))
    save = helper['write_json']
    rows, framebuffer, renderer_settings = [], None, None
    if args.resume:
        original = json.loads((out / 'manifest.json').read_text())
        for key in ['source', 'binaries', 'counts', 'trials']:
            if original[key] != manifest[key]:
                raise RuntimeError(f'resume mismatch: {key}')
        assert digest(out / 'bench-falling-cubes.py') == digest(ROOT / 'scripts/bench-falling-cubes.py'), 'measurement helper changed'
        rows = json.loads((out / 'trials.json').read_text()) if (out / 'trials.json').exists() else []
        seen = set()
        for row in rows:
            key = (row['count'], row['mode'], row['variant'], row['trial'])
            assert key not in seen and row['status'] == 'ok'
            assert key[0] in args.counts and key[1] in ['direct-gpu', 'sokol-gpu'] and key[2] in binaries and 1 <= key[3] <= args.trials
            seen.add(key)
            assert row['directory'] == f'{key[0]}-{key[1]}-{key[2]}-{key[3]}'
            raw = out / row['directory'] / f'{key[0]}-{key[1]}-1.json'
            assert digest(raw) == row['raw_sha256'], raw
            if framebuffer is None:
                framebuffer = row['framebuffer']
            assert row['framebuffer'] == framebuffer
            if row['mode'] == 'sokol-gpu':
                settings = helper['render_settings'](row['sokol_settings_after'])
                if renderer_settings is None:
                    renderer_settings = settings
                assert settings == renderer_settings
        recovery = out / 'resume-history' / datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
        recovery.mkdir(parents=True, exist_ok=False)
        save(recovery / 'manifest.json', manifest)
        save(recovery / 'previous-trials.json', rows)
        (recovery / 'runner.py').write_bytes(Path(__file__).read_bytes())
        for count in args.counts:
            for trial in range(1, args.trials + 1):
                for mode in ['direct-gpu', 'sokol-gpu']:
                    for variant in binaries:
                        if (count, mode, variant, trial) not in seen:
                            run = out / f'{count}-{mode}-{variant}-{trial}'
                            for path in [run, run.with_suffix('.log')]:
                                if path.exists():
                                    path.rename(recovery / path.name)
    else:
        out.mkdir(parents=True, exist_ok=False)
        save(out / 'manifest.json', manifest)
        (out / 'runner.py').write_bytes(Path(__file__).read_bytes())
        (out / 'bench-falling-cubes.py').write_bytes((ROOT / 'scripts/bench-falling-cubes.py').read_bytes())
    env = {k: v for k, v in os.environ.items() if not k.startswith(('GPU_PHYSICS_', 'GPU_SOKOL_', 'GPU_BENCH_'))}
    env.update(source['environment'])
    completed = {(r['count'], r['mode'], r['variant'], r['trial']) for r in rows}
    for count in args.counts:
        for trial in range(1, args.trials + 1):
            for mode in ['direct-gpu', 'sokol-gpu']:
                for variant in (['before', 'after'] if trial % 2 else ['after', 'before']):
                    if (count, mode, variant, trial) in completed:
                        continue
                    chosen = binaries[variant]
                    for identity in chosen.values():
                        assert digest(Path(identity['path'])) == identity['sha256']
                    run = out / f'{count}-{mode}-{variant}-{trial}'
                    cmd = [sys.executable, 'scripts/bench-falling-cubes.py', str(run),
                           '--counts', str(count), '--modes', mode, '--trials', '1', '--workers', '8',
                           '--gpu-solver', 'global', '--warmup', '90', '--timed', '240', '--timeout', '900',
                           '--gpu-binary', chosen['gpu']['path'], '--sokol-gpu-binary', chosen['sokol_gpu']['path']]
                    print(f'{count} {mode} {variant} trial {trial}', flush=True)
                    with run.with_suffix('.log').open('w') as log:
                        subprocess.run(cmd, cwd=ROOT, env=env, stdout=log, stderr=log, check=True)
                    result = json.loads((run / 'trials.json').read_text())
                    assert len(result) == 1 and result[0]['status'] == 'ok', result
                    row = result[0] | dict(variant=variant, trial=trial, directory=run.name)
                    if framebuffer is None:
                        framebuffer = row['framebuffer']
                    assert row['framebuffer'] == framebuffer
                    measured = json.loads((run / 'manifest.json').read_text())
                    assert measured['environment'] == source['environment']
                    if mode == 'sokol-gpu':
                        settings = helper['render_settings'](row['sokol_settings_after'])
                        if renderer_settings is None:
                            renderer_settings = settings
                        assert settings == renderer_settings
                    path = run / f'{count}-{mode}-1.json'
                    row['raw_sha256'] = digest(path)
                    rows.append(row)
                    save(out / 'trials.json', rows)
                    print(f"  {1000 / row['mean_ms']:.2f} FPS", flush=True)


if __name__ == '__main__':
    main()
