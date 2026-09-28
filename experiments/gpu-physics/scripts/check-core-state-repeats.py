#!/usr/bin/env python3
"""Fresh-process repeatability of the captured diagnostic state, not a full-state gate."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--binary', type=Path, required=True)
p.add_argument('--out', type=Path, required=True)
p.add_argument('--frames', type=int, default=600)
p.add_argument('--runs', type=int, default=5)
p.add_argument('--configuration', choices=['ordinary', 'native-cached'], required=True)
a = p.parse_args()
if a.runs < 5 or a.frames < 1:
    p.error('require at least five fresh processes and a positive frame count')
root = Path(__file__).resolve().parents[1]
out = a.out.resolve()
out.mkdir(parents=True, exist_ok=True)
if any(out.iterdir()):
    p.error('output directory must be empty; previous evidence is never overwritten')
binary = out/'fixture'
shutil.copy2(a.binary.resolve(), binary)
env = os.environ.copy()
# Eliminate inherited experimental switches; record all explicitly selected ones.
for key in list(env):
    if key.startswith(('GPU_PHYSICS_', 'WGPU_')):
        del env[key]
if a.configuration == 'native-cached':
    raw = subprocess.check_output(['bash', '-c', 'source scripts/native-samples-cache-env.sh; env -0'], cwd=root, env=env)
    env.update(item.decode().split('=', 1) for item in raw.split(b'\0') if b'=' in item)
env.update(GPU_PHYSICS_LIVE_CONTACT_ORDER='0', GPU_PHYSICS_PIPELINE_CACHE_DIR=str(Path.home()/'.cache/box3d-gpu-physics/pipelines'))
manifest = dict(binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                configuration=a.configuration, frames=a.frames, runs=a.runs,
                environment={k:v for k,v in env.items() if k.startswith(('GPU_PHYSICS_','WGPU_','VK_'))},
                scope='Captured diagnostic state only; full-state coverage audit outstanding')
(out/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
traces = []
for i in range(a.runs):
    trace = out/f'run-{i+1}.jsonl'
    env['GPU_PHYSICS_STATE_TRACE'] = str(trace)
    with (out/f'run-{i+1}.txt').open('w') as poses, (out/f'run-{i+1}.log').open('w') as log:
        result = subprocess.run([str(binary), 'ragdolls', str(a.frames)], cwd=root, env=env, stdout=poses, stderr=log)
    (out/f'run-{i+1}-exit.txt').write_text(str(result.returncode)+'\n')
    result.check_returncode()
    traces.append(trace)
    print(f'completed fresh run {i+1}/{a.runs}', flush=True)
spec = importlib.util.spec_from_file_location('compare', root/'scripts/compare-core-state.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
result = m.compare(traces, a.frames)
(out/'result.json').write_text(json.dumps(result, indent=2)+'\n')
print(json.dumps(result, indent=2))
raise SystemExit(result['status'] != 'pass')
