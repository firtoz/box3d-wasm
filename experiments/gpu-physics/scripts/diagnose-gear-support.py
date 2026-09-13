#!/usr/bin/env python3
"""Diagnostic only: native rock-vertex intrusion into Gear Lift's stairwell.

Does not certify whole convex/mesh separation, edges, swept motion, or contacts.
Does not replace the native-scene gate or mutate its acceptance criteria.
"""
import argparse
import hashlib
import json
import math
import re
import struct
from pathlib import Path
from native_scene_validate import record_complete

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--cpu', required=True, type=Path)
p.add_argument('--gpu', required=True, type=Path)
p.add_argument('--vertices', required=True, type=Path)
p.add_argument('--source', required=True, type=Path)
p.add_argument('--out', required=True, type=Path)
a = p.parse_args()
source = a.source.read_text().split('class GearLift :')[1]
block = source.split('static const b3Vec2 points[32] = {')[1].split('};')[0]
f32 = lambda x: struct.unpack('f', struct.pack('f', float(x)))[0]
polygon = [tuple(map(f32, v)) for v in re.findall(r'\{\s*([-\d.]+)f,\s*([-\d.]+)f\s*\}', block)]
assert len(polygon) == 32
assert 'float zMin = -2.0f;' in source and 'float zMax = 2.0f;' in source
assert 'int xCount = 12, yCount = 10;' in source
assert 'm_rockRadius = 0.3f;' in source
vertices = json.loads(a.vertices.read_text())
assert len(vertices) == 10 and all(len(v) == 3 for v in vertices)
assert all(math.isfinite(x) for v in vertices for x in v)

def depth(point):
    x, y, z = point
    if abs(z) >= 2:
        return 0.0
    inside, nearest = False, math.inf
    for v, w in zip(polygon, polygon[1:] + polygon[:1]):
        dx, dy = w[0] - v[0], w[1] - v[1]
        t = max(0, min(1, ((x-v[0])*dx + (y-v[1])*dy)/(dx*dx+dy*dy)))
        nearest = min(nearest, math.hypot(x-v[0]-t*dx, y-v[1]-t*dy))
        if (v[1] > y) != (w[1] > y) and x < dx*(y-v[1])/dy+v[0]:
            inside = not inside
    return min(nearest, 2-abs(z)) if inside else 0.0

def rotate(q, v):
    x, y, z, w = q
    px, py, pz = v
    tx, ty, tz = 2*(y*pz-z*py), 2*(z*px-x*pz), 2*(x*py-y*px)
    return [px+w*tx+y*tz-z*ty, py+w*ty+z*tx-x*tz, pz+w*tz+x*ty-y*tx]

# Geometry sanity checks only, not evidence of native physics correctness.
assert depth([-8, 2, 0]) > 1 and depth([0, 5, 0]) == 0 and depth([-8, 2, 3]) == 0
results = {}
matched_settings = None
matched_steps = None
for engine, path in [('cpu', a.cpu), ('gpu', a.gpu)]:
    data = json.loads(path.read_text())
    complete, detail = record_complete(data, 'Joints/Gear Lift', data['timed'])
    assert complete == 'ok', detail
    settings = [data[k] for k in ('warmup', 'timed', 'worker_count', 'enable_sleep', 'unpaced', 'completed_step_mode', 'scene_seed')]
    steps = [f['submitted_step'] for f in data['frames']]
    if matched_settings is None:
        matched_settings, matched_steps = settings, steps
    else:
        assert settings == matched_settings and steps == matched_steps, 'unmatched CPU/GPU runs'
    assert data['gear_diagnostic'] == 0 and data['measured'] == data['timed']
    frames = data['frames']
    assert frames and len(frames) == data['measured']
    # The constructor creates its 12x10 debris last. Reject mismatched layouts.
    ids = sorted(b['id'] for b in frames[0]['bodies'])
    assert len(ids) == 203 and len(set(ids)) == 203
    debris = set(ids[-120:])
    assert debris == set(range(86, 206))
    violations, center_violations, escaped = [], [], {}
    for frame in frames:
        assert sorted(b['id'] for b in frame['bodies']) == ids
        for body in frame['bodies']:
            if body['id'] not in debris:
                continue
            points = [[x+y for x, y in zip(body['p'], rotate(body['q'], v))] for v in vertices]
            d = max(depth(v) for v in points)
            record = {'step': frame['submitted_step'], 'body': body['id'], 'depth': d, 'p': body['p'], 'v': body['v']}
            if d > .02:
                violations.append(record)
            if depth(body['p']) > .02:
                center_violations.append(record)
            if min(v[2] for v in points) > 2 and body['id'] not in escaped:
                escaped[body['id']] = {'step': frame['submitted_step'], 'p': body['p']}
    results[engine] = {'vertex_intrusion_over_2cm': len(violations),
                       'first': violations[:5], 'worst': sorted(violations, key=lambda v: -v['depth'])[:5],
                       'center_intrusion_over_2cm': len(center_violations),
                       'first_fully_outside_positive_z': escaped}
report = {'status': 'diagnostic', 'results': results,
          'limitations': ['Vertex samples do not prove whole-hull separation or swept collision correctness.',
                          'Existing native screening failure remains required; no acceptance criteria changed.',
                          'Only the exact matched Gear Lift layout and recorded time window are evaluated.'],
          'sha256': {str(f): hashlib.sha256(f.read_bytes()).hexdigest() for f in [a.cpu, a.gpu, a.vertices, a.source, Path(__file__)]}}
a.out.parent.mkdir(parents=True, exist_ok=True)
a.out.write_text(json.dumps(report, indent=2) + '\n')
for engine, result in results.items():
    print(engine, 'vertex events:', result['vertex_intrusion_over_2cm'], 'center events:', result['center_intrusion_over_2cm'])
