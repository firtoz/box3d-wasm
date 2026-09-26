#!/usr/bin/env python3
"""Compare isolated twist dynamics and screen the upstream eight-ragdoll drop."""
import json
import math
import sys
from pathlib import Path

out = Path(sys.argv[1])
report = {}


def read(scene, engine, steps, bodies):
    rows = [list(map(float, line.split()))
            for line in (out / f'{scene}-{engine}.txt').read_text().splitlines()]
    assert len(rows) == (steps + 1) * bodies, (scene, engine, len(rows))
    assert all(len(row) == 16 and all(map(math.isfinite, row)) for row in rows)
    assert [(int(row[0]), int(row[1])) for row in rows] == [
        (frame, body) for frame in range(steps + 1) for body in range(bodies)]
    return rows


for scene in ['twist', 'twist-negative', 'tilted', 'tilted-negative']:
    cpu = read(scene, 'cpu', 120, 1)
    gpu = read(scene, 'gpu', 120, 1)
    error = max(abs(x - y) for a, b in zip(cpu, gpu) for x, y in zip(a[2:], b[2:]))
    assert error <= 1e-5, (scene, error)
    report[scene] = {'max_state_error': error, 'tolerance': 1e-5}

cpu = read('ragdolls', 'cpu', 600, 112)
gpu = read('ragdolls', 'gpu', 600, 112)
initial_error = max(abs(x-y) for a,b in zip(cpu[:112],gpu[:112]) for x,y in zip(a[2:],b[2:]))
airborne_position_error = max(math.dist(a[2:5], b[2:5])
                              for a,b in zip(cpu[:61*112],gpu[:61*112]))
assert initial_error <= 1e-5, initial_error
# Small pre-impact differences remain. Do not claim
# trajectory equivalence once contact-sensitive ragdoll piles diverge.
assert airborne_position_error <= .01, airborne_position_error
metrics = {}
for engine, rows in [('cpu', cpu), ('gpu', gpu)]:
    tail = rows[480*112:]
    metrics[engine] = {
        'peak_angular_speed': max(math.dist(row[12:15], [0,0,0]) for row in rows),
        'peak_joint_separation': max(row[15] for row in rows),
        'tail_linear_speed': max(math.dist(row[9:12], [0,0,0]) for row in tail),
        'tail_angular_speed': max(math.dist(row[12:15], [0,0,0]) for row in tail),
        'tail_joint_separation': max(row[15] for row in tail),
    }
    # Allow transient impact stretch, but require the pile to settle without
    # persistent joint motion during the final two seconds.
    assert metrics[engine]['peak_joint_separation'] < .25, metrics
    assert metrics[engine]['tail_joint_separation'] < .005, metrics
    assert metrics[engine]['tail_linear_speed'] < .05, metrics
    assert metrics[engine]['tail_angular_speed'] < .1, metrics
assert metrics['gpu']['peak_angular_speed'] <= 1.2 * metrics['cpu']['peak_angular_speed'], metrics
report['ragdolls'] = {'initial_error': initial_error,
                      'airborne_position_error': airborne_position_error, **metrics}
report['status'] = 'pass'
(out / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
