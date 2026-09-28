#!/usr/bin/env python3
"""Strict physical manifold comparison for identical frozen-pose fixtures.

Internal triangle/feature encodings differ between engines; compare point counts,
normals and separation multisets. This is not a chaotic trajectory comparison.
"""
import argparse
import json
import math
from pathlib import Path


def read(path, count):
    cases, manifolds = {}, {}
    for line in path.read_text().splitlines():
        row = line.split()
        identity = int(row[1])
        if row[0] == 'case':
            if identity in cases:
                raise ValueError(f'{path}: duplicate case {identity}')
            cases[identity] = tuple(map(int, row[2:]))
        elif row[0] == 'manifold':
            points = int(row[2])
            normal = list(map(float, row[3:6]))
            separations = sorted(float(x.split(':')[0]) for x in row[6:])
            if len(normal) != 3 or len(separations) != points or not all(map(math.isfinite, normal + separations)):
                raise ValueError(f'{path}: invalid manifold in case {identity}')
            manifolds.setdefault(identity, []).append(normal + separations)
        else:
            raise ValueError(f'{path}: unknown record {row[0]}')
    if set(cases) != set(range(count)) or not set(manifolds) <= set(cases):
        raise ValueError(f'{path}: incomplete or unexpected cases')
    for identity, summary in cases.items():
        points = sum(len(m)-3 for m in manifolds.get(identity, []))
        if len(summary) != 2 or points != summary[1] or bool(summary[0]) != bool(points):
            raise ValueError(f'{path}: inconsistent case {identity}')
    return cases, manifolds


def compare(cpu, gpu, count, epsilon):
    a, am = read(cpu, count)
    b, bm = read(gpu, count)
    errors, worst = [], 0.0
    for identity in a:
        if a[identity] != b[identity]:
            errors.append({'case': identity, 'counts': [a[identity], b[identity]]})
            continue
        remaining = list(bm.get(identity, []))
        for manifold in am.get(identity, []):
            matches = [(max(abs(x-y) for x, y in zip(manifold, other)), i)
                       for i, other in enumerate(remaining) if len(other) == len(manifold)]
            if not matches:
                errors.append({'case': identity, 'reason': 'missing manifold with matching point count'})
                break
            error, index = min(matches)
            remaining.pop(index)
            worst = max(worst, error)
            if error > epsilon:
                errors.append({'case': identity, 'component_error': error})
        if remaining:
            errors.append({'case': identity, 'reason': 'extra manifolds'})
    return {'status': 'fail' if errors else 'pass', 'cases': count,
            'epsilon': epsilon, 'max_matched_component_error': worst, 'failures': errors}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cpu', type=Path)
    parser.add_argument('gpu', type=Path)
    parser.add_argument('--cases', type=int, required=True)
    parser.add_argument('--epsilon', type=float, default=1e-5)
    args = parser.parse_args()
    if args.cases < 1 or not math.isfinite(args.epsilon) or args.epsilon <= 0:
        parser.error('positive case count and finite positive epsilon required')
    result = compare(args.cpu, args.gpu, args.cases, args.epsilon)
    print(json.dumps(result, indent=2))
    raise SystemExit(result['status'] != 'pass')
