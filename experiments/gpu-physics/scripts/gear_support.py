"""Gear Lift solid-support checks, independent of GPU contact generation.

Complete hull/prism SAT at recorded poses; not a swept collision certificate.
NumPy is required. Missing geometry dependencies or stale fixtures fail closed.
"""
import hashlib
import itertools
from pathlib import Path

import numpy as np
from gear_support_fixture import GEOMETRY

ROCK_IDS = frozenset(GEOMETRY['rock_ids'])


def rotation(q):
    x, y, z, w = q
    return np.array([[1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
                     [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
                     [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)]])


def gaps(points, axes, centers, half):
    projection = points @ axes.T
    middle = centers @ axes.T
    radius = half @ np.abs(axes).T
    return np.maximum(middle-radius-projection.max(0),
                      projection.min(0)-(middle+radius)).max(1)


def geometry():
    repo = Path(__file__).resolve().parents[3]
    for name, expected in GEOMETRY['sources'].items():
        if hashlib.sha256((repo/name).read_bytes()).hexdigest() != expected:
            raise ValueError(f'Gear Lift geometry fixture is stale: {name}')
    poly = np.array(GEOMETRY['silhouette'], dtype=np.float32).astype(float)
    if poly.shape != (32, 2) or any(np.count_nonzero(poly[(i+1)%32]-a) != 1 for i, a in enumerate(poly)):
        raise ValueError('invalid stair silhouette')
    boxes = []
    xs = sorted(set(poly[:, 0]))
    for lo, hi in zip(xs, xs[1:]):
        x = (lo+hi)/2
        ys = sorted(float(a[1]) for i, a in enumerate(poly)
                    if min(a[0], poly[(i+1)%32, 0]) < x < max(a[0], poly[(i+1)%32, 0]))
        if len(ys)%2:
            raise ValueError('invalid stair prism decomposition')
        boxes.extend(([lo, bottom, -2.], [hi, top, 2.]) for bottom, top in zip(ys[::2], ys[1::2]))
    boxes.extend([([-20., -2., -20.], [20., 0., 20.]),
                  ([poly[:, 0].min(), poly[:, 1].min(), -2.1],
                   [poly[:, 0].max(), poly[:, 1].max(), -2.])])
    low, high = (np.array([b[i] for b in boxes]) for i in [0, 1])
    vertices = np.array(GEOMETRY['rock_points'], dtype=float)
    faces = []
    for i, j, k in itertools.combinations(range(len(vertices)), 3):
        n = np.cross(vertices[j]-vertices[i], vertices[k]-vertices[i])
        length = np.linalg.norm(n)
        if length < 1e-9:
            continue
        n /= length
        d = (vertices-vertices[i]) @ n
        if d.max() > 1e-6 and d.min() < -1e-6:
            continue
        if not any(abs(n@m) > 1-1e-8 for m in faces):
            faces.append(n)
    # Includes every hull edge; extra chord axes cannot invent intersection.
    edges = np.array([vertices[j]-vertices[i] for i, j in itertools.combinations(range(len(vertices)), 2)])
    return vertices, np.array(faces), edges, low, high


def analyze(doc):
    if doc.get('gear_diagnostic', 0) != 0:
        raise ValueError('Gear Lift diagnostics change the fixture')
    if doc.get('warmup') != 2 or doc.get('timed', 0) < 1200:
        raise ValueError('Gear Lift requires warmup 2 and at least 1200 measured steps')
    vertices, faces, edges, low, high = geometry()
    centers, half = (low+high)/2, (high-low)/2
    rock_ids = ROCK_IDS
    last, streaks = {}, {}
    result = dict(poses=0, deep_poses=0, peak_depth=0., longest_deep=0,
                  final_deep=0, floor_crossings=0)
    for frame in doc['frames']:
        step = frame['submitted_step']
        rocks = [b for b in frame['bodies'] if b['id'] in rock_ids]
        if len(rocks) != 120 or {b['id'] for b in rocks} != rock_ids:
            raise ValueError('Gear Lift rock identity/geometry coverage changed')
        for body in rocks:
            q, p = np.array(body['q']), np.array(body['p'])
            if not np.all(np.isfinite(q)) or not np.all(np.isfinite(p)) or abs(q@q-1) > 1e-4:
                raise ValueError('invalid Gear Lift rigid transform')
            R = rotation(q)
            points = vertices @ R.T+p
            result['poses'] += 1
            if points[:, 1].max() < -.02 and np.all(points[:, [0, 2]].min(0) >= -20) and np.all(points[:, [0, 2]].max(0) <= 20):
                result['floor_crossings'] += 1
            candidates = np.where(np.all(points.max(0) >= low, axis=1) & np.all(points.min(0) <= high, axis=1))[0]
            if not len(candidates):
                continue
            crosses = np.cross((edges @ R.T)[:, None, :], np.eye(3)[None, :, :]).reshape(-1, 3)
            lengths = np.linalg.norm(crosses, axis=1)
            crosses = crosses[lengths > 1e-10]/lengths[lengths > 1e-10, None]
            axes = np.concatenate([np.eye(3), faces @ R.T, crosses])
            depth = max(0., float(-gaps(points, axes, centers[candidates], half[candidates]).min()))
            result['peak_depth'] = max(result['peak_depth'], depth)
            if depth > .02:
                identity = body['id']
                streaks[identity] = streaks.get(identity, 0)+1 if last.get(identity) == step-1 else 1
                last[identity] = step
                result['longest_deep'] = max(result['longest_deep'], streaks[identity])
                result['deep_poses'] += 1
                result['final_deep'] += step == doc['frames'][-1]['submitted_step']
    return result


def compare_metrics(gpu, cpu):
    if gpu['poses'] != cpu['poses'] or gpu['poses'] == 0:
        return 'fail', 'unmatched Gear Lift geometry coverage'
    for name, data in [('CPU', cpu), ('GPU', gpu)]:
        if data['floor_crossings'] or data['final_deep']:
            return 'fail', f'{name} lost ground support or ends inside the solid'
    # No tolerance increase: depth threshold remains 0.02m. Only peak comparison
    # has one native linear slop (0.005m) for the differing floating arithmetic.
    if gpu['peak_depth'] > cpu['peak_depth']+.005:
        return 'fail', 'Gear Lift peak solid penetration exceeds CPU plus linear slop'
    if gpu['longest_deep'] > max(1, cpu['longest_deep']):
        return 'fail', 'Gear Lift deep penetration persists longer than CPU'
    if gpu['deep_poses'] > cpu['deep_poses']:
        return 'fail', 'Gear Lift deep penetration is more frequent than CPU'
    return 'ok', 'Gear Lift complete hull/solid support and CPU penetration bounds'


def compare(gpu, cpu):
    try:
        return compare_metrics(analyze(gpu), analyze(cpu))
    except (ValueError, KeyError, OSError) as exc:
        return 'fail', str(exc)
