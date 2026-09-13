#!/usr/bin/env python3
"""Run cold-start Gear Lift impacts; reject mismatched initial states or incomplete traces."""
import argparse
import hashlib
import json
import math
import os
import subprocess
from pathlib import Path

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('directory', type=Path)
p.add_argument('--cpu', required=True, type=Path)
p.add_argument('--gpu', required=True, type=Path)
p.add_argument('--steps', default=12, type=int)
a = p.parse_args()
assert 1 <= a.steps <= 120
out = a.directory.resolve()
results = []
for state in sorted(out.glob('*.state')):
    lines = state.read_text().splitlines()
    initial = {int(r[0]): list(map(float, r[1:])) for r in map(str.split, lines[1:])}
    assert len(initial) == int(lines[0])
    traces = {}
    for engine, binary in [('cpu', a.cpu.resolve()), ('gpu', a.gpu.resolve())]:
        path = out / f'{state.stem}-{engine}.txt'
        with path.open('w') as stdout, path.with_suffix('.log').open('w') as stderr:
            subprocess.run([str(binary), str(state), str(a.steps)], stdout=stdout,
                           stderr=stderr, timeout=120, check=True)
        log = path.with_suffix('.log').read_text()
        assert not any(marker in log for marker in ('uncaptured WebGPU error', 'Validation Error', 'overflow true')), (path, 'invalid GPU execution')
        frames = {}
        for line in path.read_text().splitlines():
            row = line.split()
            assert len(row) == 15, (path, line)
            step, body = map(int, row[:2])
            values = list(map(float, row[2:]))
            assert all(map(math.isfinite, values))
            assert body in initial and (step, body) not in frames
            frames[step, body] = values
        assert set(frames) == {(s, b) for s in range(a.steps + 1) for b in initial}
        error = max(abs(x-y) for b in initial for x,y in zip(initial[b], frames[0,b]))
        assert error < 2e-6, (state, engine, 'initial state mismatch', error)
        traces[engine] = frames
        results.append({'state': state.name, 'engine': engine, 'initial_max_error': error,
                        'frames': len(frames), 'exit_code': 0,
                        'max_linear_speed': max(math.sqrt(sum(x*x for x in v[7:10])) for v in frames.values())})
    worst = max((math.dist(traces['cpu'][key][:3], traces['gpu'][key][:3]), key) for key in traces['cpu'])
    results.append({'state': state.name, 'max_cpu_gpu_position_delta': worst[0],
                    'worst_step_body': worst[1]})
paths = [Path(__file__).resolve(), Path(__file__).resolve().parents[1] / 'c_abi/gear_impact_replay.cpp',
         out/'gear_terrain_generated.inc', a.cpu.resolve(), a.gpu.resolve(), *sorted(out.glob('*.state'))]
report = {'status': 'diagnostic', 'steps': a.steps, 'results': results,
          'environment': {k: os.environ.get(k) for k in ('GPU_PHYSICS_AB','GEAR_REPLAY_NO_CCD')},
          'limitations': ['Cold-start replay omits mechanism bodies, joints and previous contact caches.',
                          'Trajectory differences and finite states do not certify stable contact or native compatibility.'],
          'sha256': {str(f): hashlib.sha256(f.read_bytes()).hexdigest() for f in paths}}
(out/'runs.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(results, indent=2))
