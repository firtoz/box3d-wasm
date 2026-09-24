#!/usr/bin/env python3
"""Measure real sample Shift-clicks; use a prebuilt executable and launcher env."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--binary', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--adapter', choices=['amd', 'nvidia'], required=True)
p.add_argument('--samples', nargs='+', default=['Stacking/Box Stack', 'Stacking/Dominoes',
    'GPU Bench/Mixed Stacks 4096', 'Mesh/Grid', 'Joints/Revolute', 'Compound/Village'])
p.add_argument('--shots', default='20,40,60')
a = p.parse_args()
shots = [int(x) for x in a.shots.split(',')]
assert shots and min(shots) >= 10 and len(set(shots)) == len(shots)
binary = a.binary.resolve()
out = a.output.resolve(); out.mkdir(parents=True, exist_ok=False)
env = os.environ | {'GPU_PHYSICS_ADAPTER': a.adapter, 'GPU_SOKOL_SHOOT_FRAMES': a.shots}
manifest = {'binary': str(binary), 'sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
    'shots': shots, 'samples': a.samples, 'environment': {k: v for k, v in env.items()
        if k.startswith(('GPU_', 'WGPU_', 'VK_', '__NV_', '__GLX_'))}}
(out/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n')
summary = []
for index, sample in enumerate(a.samples):
    name = f'{index:02d}'
    print(sample, flush=True)
    cmd = [str(binary), '--sample-name', sample, '--warmup', '10', '--timed', str(max(shots)+10),
        '--unpaced', '--bench-json', str(out/(name+'.json'))]
    with (out/(name+'.log')).open('w') as log:
        subprocess.run(cmd, cwd=ROOT.parents[1]/'box3d', env=env, stdout=log, stderr=log,
            check=True, timeout=300)
    data = json.loads((out/(name+'.json')).read_text())
    assert data['status'] == 'ok' and not data['gpu_fail'], data
    frames = data['frames']
    assert not any(f['gpu_contact_metrics']['capacity_loss'] for f in frames), sample
    measured = []
    for shot in shots:
        # i is zero-based within the timed interval (after ten warmup frames).
        row = frames[shot-10]; previous = frames[shot-11]; following = frames[shot-9]
        assert row['body_count'] == previous['body_count']+1, (sample, shot, 'handler did not add one body')
        assert row['completed_step'] == row['submitted_step'] == row['rendered_pose'], row
        measured.append({'frame': shot, 'physics_ms': row['physics_ms'],
            'following_cadence_ms': following['cadence_ms'], 'body_count': row['body_count']})
    summary.append({'sample': sample, 'steady_physics_p50_ms': data['physics_p50_ms'], 'shots': measured})
    (out/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print(measured, flush=True)
