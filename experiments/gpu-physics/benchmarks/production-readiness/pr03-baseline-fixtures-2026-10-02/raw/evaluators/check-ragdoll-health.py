#!/usr/bin/env python3
"""Physical ragdoll screen; retains existing limits without trajectory equality."""
import argparse
import hashlib
import json
import math
from pathlib import Path


LIMITS = {
    'peak_joint_separation': 0.25,
    'tail_joint_separation': 0.005,
    'tail_linear_speed': 0.05,
    'tail_angular_speed': 0.1,
}


def measure(path):
    metrics = {key: {'value': -math.inf, 'frame': None, 'body': None}
               for key in [*LIMITS, 'peak_angular_speed']}
    count = 0
    with path.open() as source:
        for count, line in enumerate(source, 1):
            row = list(map(float, line.split()))
            if len(row) != 16 or not all(map(math.isfinite, row)):
                raise ValueError(f'{path}: invalid/nonfinite row {count}')
            frame, body = divmod(count - 1, 112)
            if row[:2] != [frame, body]:
                raise ValueError(f'{path}: incorrect frame/body at row {count}')
            values = {'peak_joint_separation': row[15],
                      'peak_angular_speed': math.hypot(*row[12:15])}
            if frame >= 480:
                values.update(tail_joint_separation=row[15],
                              tail_linear_speed=math.hypot(*row[9:12]),
                              tail_angular_speed=math.hypot(*row[12:15]))
            for key, value in values.items():
                if value > metrics[key]['value']:
                    metrics[key] = dict(value=value, frame=frame, body=body)
    if count != 601 * 112:
        raise ValueError(f'{path}: expected 67312 rows (frames 0..600), got {count}')
    return metrics


def evaluate(cpu, gpu):
    failures = []
    for engine, metrics in [('cpu', cpu), ('gpu', gpu)]:
        for key, limit in LIMITS.items():
            if metrics[key]['value'] >= limit:
                failures.append(dict(engine=engine, metric=key, limit=limit,
                                     **metrics[key]))
    limit = 1.2 * cpu['peak_angular_speed']['value']
    if gpu['peak_angular_speed']['value'] > limit:
        failures.append(dict(engine='gpu', metric='peak_angular_speed',
                             limit=limit, **gpu['peak_angular_speed']))
    return dict(status='fail' if failures else 'pass', failures=failures,
                limits=LIMITS, peak_angular_speed_cpu_multiplier=1.2,
                cpu=cpu, gpu=gpu,
                scope='Physical screen only; not complete-state repeatability or full collision/joint correctness')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cpu', type=Path, required=True)
    parser.add_argument('--gpu', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    result = evaluate(measure(args.cpu), measure(args.gpu))
    result['inputs'] = {name: dict(path=str(path), sha256=hashlib.sha256(path.read_bytes()).hexdigest())
                        for name, path in [('cpu', args.cpu), ('gpu', args.gpu)]}
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))
    return result['status'] != 'pass'


if __name__ == '__main__':
    raise SystemExit(main())
