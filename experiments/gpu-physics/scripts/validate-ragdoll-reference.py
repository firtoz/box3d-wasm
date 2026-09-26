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

cpu = read('ragdolls-no-contacts', 'cpu', 60, 112)
gpu = read('ragdolls-no-contacts', 'gpu', 60, 112)
position_error = max(math.dist(a[2:5], b[2:5]) for a, b in zip(cpu, gpu))
# Creation-order joint solving produced ~4 mm of error in the first step.
# Allow floating-point drift, but not a different constraint solve sequence.
assert position_error <= 5e-5, position_error
report['ragdolls-no-contacts'] = {'max_position_error': position_error, 'tolerance': 5e-5}

# Compare the first spine's complete frame-2 contact set against the independent
# oracle. Wrong relative-origin/bounds/order recycled frame 1's thigh normals.
def spine_contacts(engine):
    contacts = {}
    for line in (out / f'ragdolls-{engine}.log').read_text().splitlines():
        fields = line.split()
        if fields[:1] != ['spine-contact']:
            continue
        assert len(fields) == 8, fields
        key = tuple(map(int, fields[1:5]))
        assert key not in contacts, key
        normal = list(map(float, fields[5:8]))
        assert all(map(math.isfinite, normal)), normal
        contacts[key] = normal
    assert len(contacts) == 3, (engine, contacts)
    return contacts

cpu_contacts, gpu_contacts = spine_contacts('cpu'), spine_contacts('gpu')
assert cpu_contacts.keys() == gpu_contacts.keys(), (cpu_contacts, gpu_contacts)
contact_normal_error = max(math.dist(normal, gpu_contacts[key])
                           for key, normal in cpu_contacts.items())
assert contact_normal_error <= 1e-4, contact_normal_error
report['frame-2-spine-contacts'] = {'max_normal_error': contact_normal_error,
                                    'tolerance': 1e-4}

cpu = read('ragdolls', 'cpu', 600, 112)
gpu = read('ragdolls', 'gpu', 600, 112)
initial_error = max(abs(x-y) for a,b in zip(cpu[:112],gpu[:112]) for x,y in zip(a[2:],b[2:]))
airborne_position_error = max(math.dist(a[2:5], b[2:5])
                              for a,b in zip(cpu[:61*112],gpu[:61*112]))
assert initial_error <= 1e-5, initial_error
# Small pre-impact differences remain. Do not claim
# trajectory equivalence once contact-sensitive ragdoll piles diverge.
assert airborne_position_error <= .006, airborne_position_error
first_step_error = max(math.dist(a[2:5], b[2:5])
                       for a, b in zip(cpu[112:224], gpu[112:224]))
assert first_step_error <= 2e-4, first_step_error
# Evaluate the visual-agreement goal at EVERY frame, not only checkpoints.
def angular_error_degrees(a, b):
    # Normalize the printed quaternions and identify q with -q. The chord form
    # avoids loss of precision in acos(dot) for almost identical orientations.
    na = math.sqrt(sum(x*x for x in a))
    nb = math.sqrt(sum(x*x for x in b))
    assert na > 0 and nb > 0, (a, b)
    a = [x / na for x in a]
    b = [x / nb for x in b]
    chord = min(math.dist(a, b), math.dist(a, [-x for x in b]))
    return math.degrees(4 * math.asin(min(1, chord / 2)))

limits = {'rms_position_error': .01, 'max_position_error': .05,
          'rms_orientation_error_degrees': 1, 'max_orientation_error_degrees': 5}
frames = []
for frame in range(601):
    pairs = list(zip(cpu[frame*112:(frame+1)*112], gpu[frame*112:(frame+1)*112]))
    position = [math.dist(a[2:5], b[2:5]) for a, b in pairs]
    orientation = [angular_error_degrees(a[5:9], b[5:9]) for a, b in pairs]
    frames.append({'frame': frame,
                   'max_position_error': max(position),
                   'worst_position_body': position.index(max(position)),
                   'rms_position_error': math.sqrt(sum(e*e for e in position) / 112),
                   'max_orientation_error_degrees': max(orientation),
                   'worst_orientation_body': orientation.index(max(orientation)),
                   'rms_orientation_error_degrees': math.sqrt(sum(e*e for e in orientation) / 112)})
violations = [row for row in frames if any(row[key] > limit for key, limit in limits.items())]
visual = {'status': 'fail' if violations else 'pass', 'limits': limits,
          'first_failing_frame': violations[0] if violations else None,
          'failing_frame_count': len(violations),
          'worst': {key: max(frames, key=lambda row: row[key]) for key in limits}}
(out / 'frame-errors.json').write_text(json.dumps(frames, indent=2) + '\n')
checkpoints = {frame: frames[frame] for frame in [1, 60, 87, 88, 100, 143, 300, 600]}
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
                      'airborne_position_error': airborne_position_error,
                      'checkpoints': checkpoints, 'visual_agreement': visual, **metrics}
report['stability_status'] = 'pass'
report['status'] = visual['status']
(out / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
assert not violations, ('visual agreement goal failed', visual['first_failing_frame'])
