#!/usr/bin/env python3
"""Check loss/error/completion status at every captured step, not physical correctness."""
import argparse
import gzip
import json
from pathlib import Path


def check_frame(frame, number):
    if frame.get('schema') not in ('gpu-core-state-v19','gpu-core-state-v20','gpu-core-state-v21','gpu-core-state-v22') or frame.get('frame') != number:
        raise ValueError(f'frame {number}: unsupported schema or nonsequential frame')
    policy, idle = frame['gpu_policy'], frame['idle_state']
    expected = {
        'completed_known': True, 'completed_step': number,
        'callback_open': False, 'sticky_loss': False,
        'sticky_causes': [0] * 5, 'sticky_contact_reasons': 0,
        'sticky_first_step': 0,
    }
    for key, value in expected.items():
        actual = policy.get(key)
        if type(actual) is not type(value) or actual != value:
            raise ValueError(f'frame {number}: {key}={actual!r}, expected {value!r}')
    if idle.get('physics_invalid') is not False or idle.get('physics_step') != number:
        raise ValueError(f'frame {number}: invalid physics or completion mismatch')


def check(path, expected_frames):
    opener = gzip.open if path.suffix == '.gz' else open
    count = 0
    with opener(path, 'rt') as source:
        for count, line in enumerate(source, 1):
            check_frame(json.loads(line), count)
    if count != expected_frames:
        raise ValueError(f'expected {expected_frames} frames, found {count}')
    return {'status': 'pass', 'frames': count, 'input': str(path),
            'scope': 'No captured capacity loss, invalid-state flag or completion mismatch; not physical or full-state qualification'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('trace', type=Path)
    parser.add_argument('--frames', type=int, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    if args.frames < 1:
        parser.error('--frames must be positive')
    try:
        result = check(args.trace, args.frames)
    except (ValueError, KeyError, TypeError) as error:
        result = {'status': 'fail', 'error': str(error), 'input': str(args.trace)}
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))
    raise SystemExit(result['status'] != 'pass')
