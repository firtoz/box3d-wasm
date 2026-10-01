#!/usr/bin/env python3
"""Resumable fresh-process native-sample captures; state coverage remains audited separately."""
import argparse
import fcntl
import gzip
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def save(path, value):
    tmp = path.with_suffix(path.suffix + '.tmp')
    with tmp.open('w') as f:
        json.dump(value, f, indent=2)
        f.write('\n')
        f.flush()
        os.fsync(f.fileno())
    tmp.replace(path)


def record_child_exit(receipt, command, lock_fd):
    """Run inside Xvfb and persist the sample status before launcher cleanup."""
    start = time.monotonic()
    result = subprocess.run(command, pass_fds=(lock_fd,))
    save(receipt, {'exit': result.returncode, 'seconds': time.monotonic() - start})
    # Preserve the exact signal status in the receipt, even though a wrapper's
    # process exit cannot represent subprocess's negative signal convention.
    return result.returncode if result.returncode >= 0 else 128 - result.returncode


def successful_exit(attempt):
    receipt = json.loads((attempt / 'exit.json').read_text())
    if receipt['exit'] != 0:
        raise ValueError(f'{attempt}: recorded fixture/launcher failure; preserve this attempt')
    # Legacy receipts are still readable; new batches freeze the changed runner
    # hash and cannot resume an old batch under these new receipt semantics.
    if 'child_exit' in receipt:
        child = json.loads((attempt / 'child-exit.json').read_text())
        if child['exit'] != 0 or receipt['child_exit'] != 0 or receipt['launcher_exit'] != 0:
            raise ValueError(f'{attempt}: inconsistent successful exit receipt')
    return receipt


def validate_trace(path, steps):
    h = hashlib.sha256()
    count = 0
    opener = gzip.open if path.suffix == '.gz' else Path.open
    with opener(path, 'rb') as f:
        for count, line in enumerate(f, 1):
            h.update(line)
            frame = json.loads(line)
            if frame.get('schema') not in ('gpu-core-state-v19','gpu-core-state-v20','gpu-core-state-v21','gpu-core-state-v22','gpu-core-state-v23','gpu-core-state-v24') or frame.get('frame') != count:
                raise ValueError(f'{path}: invalid frame/schema at {count}')
            if any(not isinstance(frame.get(k), list) for k in ['bodies', 'joints', 'contacts']):
                raise ValueError(f'{path}: missing physical state at {count}')
    if count != steps:
        raise ValueError(f'{path}: expected {steps} frames, got {count}')
    return h.hexdigest()


def completed_trace(attempt, steps):
    marker = attempt / 'complete.json'
    if not marker.exists():
        return None
    m = json.loads(marker.read_text())
    trace = attempt / 'state.jsonl.gz'
    if m['steps'] != steps or m['exit'] != 0:
        raise ValueError(f'{attempt}: invalid completion marker')
    for name, expected in m['file_sha256'].items():
        if digest(attempt / name) != expected:
            raise ValueError(f'{attempt}: changed completed evidence {name}')
    receipt = successful_exit(attempt)
    required = {'state.jsonl.gz', 'health.json', 'exit.json'}
    if 'child_exit' in receipt:
        required.add('child-exit.json')
    if set(m['file_sha256']) != required:
        raise ValueError(f'{attempt}: incomplete evidence manifest')
    # CRC + exact decompressed hash detect corrupted/truncated compression.
    if validate_trace(trace, steps) != m['uncompressed_sha256']:
        raise ValueError(f'{attempt}: decompressed hash mismatch')
    return trace


def finalize_attempt(attempt, steps):
    receipt = successful_exit(attempt)
    trace = attempt / 'state.jsonl'
    raw_hash = validate_trace(trace, steps)
    health = json.loads((attempt / 'health.json').read_text())
    if len(health.get('frames', [])) != steps:
        raise ValueError(f'{attempt}: incomplete health capture')
    compressed = attempt / 'state.jsonl.gz'
    with trace.open('rb') as inp, gzip.open(compressed, 'wb', compresslevel=1) as dest:
        shutil.copyfileobj(inp, dest, 1024 * 1024)
    if validate_trace(compressed, steps) != raw_hash:
        raise ValueError(f'{attempt}: compression mismatch')
    evidence = [compressed, attempt / 'health.json', attempt / 'exit.json']
    if 'child_exit' in receipt:
        evidence.append(attempt / 'child-exit.json')
    save(attempt / 'complete.json', {'steps': steps, 'exit': 0, 'uncompressed_sha256': raw_hash,
         'file_sha256': {p.name: digest(p) for p in evidence}})
    trace.unlink()
    return compressed


def run(args):
    root = Path(__file__).resolve().parents[1]
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    lock = (out / 'batch.lock').open('a')
    fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    env = {k: v for k, v in os.environ.items() if not k.startswith(('GPU_PHYSICS_', 'WGPU_'))}
    if args.configuration == 'native-cached':
        raw = subprocess.check_output(['bash', '-c', 'source scripts/native-samples-cache-env.sh; env -0'], cwd=root, env=env)
        env.update(item.decode().split('=', 1) for item in raw.split(b'\0') if b'=' in item)
    env.update(GPU_PHYSICS_LIVE_CONTACT_ORDER='0', GPU_PHYSICS_LIFETIME_TRACE='1',
               GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home() / '.cache/box3d-gpu-physics/pipelines'),
               LIBGL_ALWAYS_SOFTWARE='1', GPU_BENCH_WIDTH='640', GPU_BENCH_HEIGHT='360')
    config = {'binary_sha256': digest(args.binary.resolve()), 'configuration': args.configuration,
              'sample': args.sample, 'steps': args.steps, 'runs': args.runs,
              'runner_sha256': digest(Path(__file__)),
              'environment': {k: v for k, v in env.items() if k.startswith(('GPU_', 'WGPU_', 'VK_', 'LIBGL', 'MESA_', 'DRI_'))},
              'scope': 'All fields of the input capture schema; persistent-state coverage and physical acceptance audited separately.'}
    manifest = out / 'manifest.json'
    binary = out / 'fixture'
    if manifest.exists():
        if json.loads(manifest.read_text()) != config or digest(binary) != config['binary_sha256']:
            raise ValueError('Build/configuration/runner changed; use a new batch directory')
    else:
        if any(p.name != 'batch.lock' for p in out.iterdir()):
            raise ValueError('Unrecognized nonempty output directory; preserve it and use a new one')
        shutil.copy2(args.binary.resolve(), binary)
        save(manifest, config)
    traces = []
    for number in range(1, args.runs + 1):
        slot = out / f'run-{number}'
        slot.mkdir(exist_ok=True)
        attempts = sorted(slot.glob('attempt-*'))
        completed = [t for a in attempts if (t := completed_trace(a, args.steps)) is not None]
        if len(completed) > 1:
            raise ValueError(f'{slot}: multiple completed attempts')
        if completed:
            traces.append(completed[0])
            print(f'reused verified run {number}', flush=True)
            continue
        for a in attempts:
            if (a / 'exit.json').exists() and json.loads((a / 'exit.json').read_text())['exit'] != 0:
                raise ValueError(f'{a}: recorded fixture failure; diagnose before creating a new candidate batch')
        recoverable = [a for a in attempts if (a / 'exit.json').exists()]
        if recoverable:
            # A successful process may have been interrupted during compression.
            # Finish validating its existing full trace instead of rerunning physics.
            if len(recoverable) != 1:
                raise ValueError(f'{slot}: ambiguous successful incomplete attempts')
            traces.append(finalize_attempt(recoverable[0], args.steps))
            print(f'recovered completed process {number}', flush=True)
            continue
        # Incomplete attempts remain untouched. Each replacement starts a new process at frame1.
        attempt = slot / f'attempt-{len(attempts) + 1:04d}'
        attempt.mkdir()
        trace = attempt / 'state.jsonl'
        env['GPU_PHYSICS_STATE_TRACE'] = str(trace)
        cmd = ['xvfb-run', '-a', '-s', '-screen 0 1920x1080x24',
               sys.executable, str(Path(__file__).resolve()), '--record-child-exit',
               str(attempt / 'child-exit.json'), str(lock.fileno()), str(binary),
               '--sample-name', args.sample, '--unpaced', '--health-scan', '--warmup', '0',
               '--timed', str(args.steps), '--bench-json', str(attempt / 'health.json')]
        save(attempt / 'command.json', cmd)
        start = time.monotonic()
        with (attempt / 'run.log').open('w') as log:
            # Keep the lock held by the child if this runner dies, preventing duplicate live runs.
            result = subprocess.run(cmd, cwd=root, env=env, stdout=log, stderr=log, pass_fds=(lock.fileno(),))
        child_path = attempt / 'child-exit.json'
        child_exit = json.loads(child_path.read_text())['exit'] if child_path.exists() else None
        # A cleanup failure is distinct from a sample failure, but remains a
        # failed attempt. Never infer child success from complete-looking output.
        status = result.returncode or (child_exit if child_exit is not None else 1)
        save(attempt / 'exit.json', {'exit': status, 'child_exit': child_exit,
             'launcher_exit': result.returncode, 'seconds': time.monotonic() - start})
        successful_exit(attempt)
        compressed = finalize_attempt(attempt, args.steps)
        traces.append(compressed)
        print(f'completed and verified fresh run {number}/{args.runs}', flush=True)
    spec = importlib.util.spec_from_file_location('compare', root / 'scripts/compare-core-state.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    for label, audit in [('raw', False), ('audited', True)]:
        result = module.compare(traces, args.steps, audited_storage=audit)
        save(out / f'{label}-result.json', result)
        print(label, result, flush=True)
    return int(json.loads((out / 'raw-result.json').read_text())['status'] != 'pass')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--configuration', choices=['ordinary', 'native-cached'], required=True)
    p.add_argument('--sample', default='Benchmark/Rain')
    p.add_argument('--steps', type=int, default=600)
    p.add_argument('--runs', type=int, default=5)
    args = p.parse_args()
    if args.steps < 1 or args.runs < 5:
        p.error('require positive steps and at least five fresh runs')
    return run(args)


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1] == '--record-child-exit':
        raise SystemExit(record_child_exit(Path(sys.argv[2]), sys.argv[4:], int(sys.argv[3])))
    raise SystemExit(main())
