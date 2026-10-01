#!/usr/bin/env python3
"""Five fresh full diagnostic-state captures for the bounded speculative-control fixture."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SELECTOR = 'api::world::speculative_policy_tests::speculative_mesh_controls_capture_complete_step_state'
FRAMES = 110


def save(path, value):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n')
    temporary.replace(path)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def module(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'scripts' / (name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--configuration', choices=['ordinary', 'native'], required=True)
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    out = a.out.resolve()
    out.mkdir(parents=True)  # Refuse to overwrite any prior result.
    binary = out / 'fixture'
    shutil.copy2(a.binary, binary)
    env = {key: value for key, value in os.environ.items() if not key.startswith(('GPU_PHYSICS_', 'WGPU_'))}
    if a.configuration == 'native':
        policy = subprocess.check_output(['bash', '-c', 'source scripts/native-samples-cache-env.sh; env -0'], cwd=ROOT, env=env)
        env.update(item.decode().split('=', 1) for item in policy.split(b'\0') if b'=' in item)
    env.update(GPU_PHYSICS_ADAPTER='nvidia', GPU_PHYSICS_BACKEND='vulkan', WGPU_BACKEND='vulkan',
               GPU_PHYSICS_LIVE_CONTACT_ORDER='1',
               GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out.parent / 'pipelines'))
    receipt = {'configuration': a.configuration, 'frames': FRAMES, 'fresh_process_budget': 5,
               'selector': SELECTOR, 'binary_sha256': sha(binary),
               'environment': {key: value for key, value in sorted(env.items()) if key.startswith(('GPU_PHYSICS_', 'WGPU_'))},
               'scope': 'Exact v23 captured semantic state for this control fixture only; final release whole-matrix PR07 remains open',
               'runs': []}
    save(out / 'receipt.json', receipt)
    health = module('check-core-state-health')
    compare = module('compare-core-state')
    traces = []
    for trial in range(1, 6):
        trace = out / f'trial-{trial}.jsonl'
        env['GPU_PHYSICS_TEST_TRACE'] = str(trace)
        cmd = [str(binary), SELECTOR, '--exact', '--nocapture', '--test-threads=1']
        with (out / f'trial-{trial}.stdout').open('w') as s, (out / f'trial-{trial}.stderr').open('w') as e:
            proc = subprocess.Popen(cmd, cwd=ROOT, env=env, stdout=s, stderr=e)
            receipt['running'] = {'pid': proc.pid, 'trial': trial, 'command': cmd}
            save(out / 'receipt.json', receipt)
            code = proc.wait()
        receipt.pop('running', None)
        result = {'trial': trial, 'exit': code}
        receipt['runs'].append(result)
        save(out / 'receipt.json', receipt)
        if code:
            raise SystemExit('Physical/assertion failure retained; no further repeats')
        result['health'] = health.check(trace, FRAMES)
        result['trace_sha256'] = sha(trace)
        traces.append(trace)
        save(out / 'receipt.json', receipt)
        print(a.configuration, trial, 'pass', flush=True)
    result = compare.compare(traces, FRAMES)
    save(out / 'comparison.json', result)
    print(json.dumps(result), flush=True)
    if result['status'] != 'pass':
        raise SystemExit('Captured-state mismatch retained; no extension')


if __name__ == '__main__':
    main()
