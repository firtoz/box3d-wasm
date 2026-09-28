#!/usr/bin/env python3
"""Screen Rain residuals using recorded creation identities across slot reuse.

The existing +0.05m/+0.05rad CPU-relative budgets are diagnostic screens, not
proof that different chaotic trajectories are physically invalid.
"""
import argparse
import json
import math
from itertools import zip_longest
from pathlib import Path
from health_identity import body_key, creation_body_map, match_joints
from native_scene_validate import generic_health


def records(path, frames):
    with path.open() as f:
        prefix = []
        for line in f:
            if line.strip() == '"frames": [':
                break
            prefix.append(line)
        else:
            raise ValueError('missing frames')
        header = json.loads(''.join(prefix) + '"frames": []\n}')
        if header['sample'] != 'Benchmark/Rain' or header['status'] != 'ok':
            raise ValueError('wrong scene or incomplete run')
        if (header['warmup'], header['timed'], header['measured'], header['frames_observed']) != (0, frames, frames, frames):
            raise ValueError('wrong step window')
        count = 0
        for line in f:
            if line.strip() == ']':
                break
            frame = json.loads(line.strip().removesuffix(','))
            if frame['i'] != count or frame['submitted_step'] != count + 1:
                raise ValueError('non-contiguous steps')
            status, detail = generic_health(dict(header, warmup=count, timed=1, measured=1,
                                                  frames=[dict(frame, i=0)]))
            if status != 'ok':
                raise ValueError(f'{path} frame {count}: {detail}')
            yield frame
            count += 1
        if count != frames or f.read().strip() != '}':
            raise ValueError('truncated or unexpected trailing data')


def spherical_limits(joint, required=False):
    if 'spherical_limits' not in joint:
        if required:
            raise ValueError('missing spherical limit capture')
        return None
    value = joint['spherical_limits']
    if joint['type'] != 6:
        if value is not None:
            raise ValueError('spherical measurement on another joint type')
        return None
    if not isinstance(value, dict):
        raise ValueError('missing spherical joint measurement')
    for field in ['cone_enabled', 'twist_enabled']:
        if type(value.get(field)) is not bool:
            raise ValueError('invalid spherical enabled flag')
    for field in ['cone_limit', 'lower_twist', 'upper_twist', 'swing', 'twist',
                  'cone_excess', 'lower_twist_excess', 'upper_twist_excess']:
        number = value.get(field)
        if type(number) not in (int, float) or not math.isfinite(number):
            raise ValueError('invalid spherical measurement: '+field)
    if value['cone_limit'] < 0 or value['swing'] < 0 or value['lower_twist'] > value['upper_twist']:
        raise ValueError('invalid spherical limit range')
    expected = {
        'cone_excess': max(0, value['swing']-value['cone_limit']) if value['cone_enabled'] else 0,
        'lower_twist_excess': max(0, value['lower_twist']-value['twist']) if value['twist_enabled'] else 0,
        'upper_twist_excess': max(0, value['twist']-value['upper_twist']) if value['twist_enabled'] else 0,
    }
    for field, computed in expected.items():
        if value[field] < 0 or not math.isclose(value[field], computed, abs_tol=1e-6, rel_tol=1e-6):
            raise ValueError('inconsistent spherical violation: '+field)
    return value


def joint_measurements(cpu, gpu, require_spherical=False):
    """Yield named (GPU, CPU) residuals after validating matching semantics."""
    sa, sb = spherical_limits(cpu, require_spherical), spherical_limits(gpu, require_spherical)
    if ('spherical_limits' in cpu) != ('spherical_limits' in gpu) or (sa is None) != (sb is None):
        raise ValueError('spherical capture coverage differs')
    if sa is not None:
        for field in ['cone_enabled', 'twist_enabled', 'cone_limit', 'lower_twist', 'upper_twist']:
            if sa[field] != sb[field]:
                raise ValueError('spherical constraint parameters differ: '+field)
        for field in ['cone_excess', 'lower_twist_excess', 'upper_twist_excess']:
            yield field, sb[field], sa[field]
    for field in ['anchor', 'angular']:
        if cpu[field+'_constrained'] != gpu[field+'_constrained']:
            raise ValueError('constraint semantics differ')
        if gpu[field+'_constrained']:
            yield field, gpu[field+'_error'], cpu[field+'_error']


def compare(cpu, gpu, frames, require_spherical=False):
    report = dict(frames=0, matched_joint_observations=0, anchor_failures=0,
                  angular_failures=0, first_failures=[], peak_anchor={}, peak_angular={},
                  scope='Creation-matched Rain residual screening; not full-state determinism')
    report.update(spherical_observations=0, cone_excess_failures=0,
                  lower_twist_excess_failures=0, upper_twist_excess_failures=0,
                  peak_cone_excess={}, peak_lower_twist_excess={}, peak_upper_twist_excess={})
    lifetimes = [{}, {}]
    slots = [{}, {}]
    reused = [set(), set()]
    for c, g in zip_longest(records(cpu, frames), records(gpu, frames)):
        if c is None or g is None or c['i'] != g['i']:
            raise ValueError('mismatched windows')
        for side, frame in enumerate([c, g]):
            for b in frame['bodies']:
                creation, key = b['creation'], body_key(b)
                if creation in lifetimes[side] and lifetimes[side][creation] != key:
                    raise ValueError('creation identity changed body lifetime')
                lifetimes[side][creation] = key
                if key[0] in slots[side] and slots[side][key[0]] != creation:
                    reused[side].add(key[0])
                slots[side][key[0]] = creation
        mapping = creation_body_map(c['bodies'], g['bodies'])
        pairs = match_joints(c['joints'], g['joints'], mapping)
        creations = {body_key(b): b['creation'] for b in g['bodies']}
        for a, b in pairs:
            identity = dict(type=b['type'], endpoints=[creations[body_key(b, 'body_a')], creations[body_key(b, 'body_b')]])
            for field, value, reference in joint_measurements(a, b, require_spherical):
                if field == 'cone_excess':
                    report['spherical_observations'] += 1
                item = dict(frame=g['i'], gpu=value, cpu=reference, **identity)
                peak = 'peak_'+field
                if not report[peak] or value > report[peak]['gpu']:
                    report[peak] = item
                if value > reference + 0.05:
                    report[field+'_failures'] += 1
                    if len(report['first_failures']) < 20:
                        report['first_failures'].append(dict(field=field, **item))
        report['matched_joint_observations'] += len(pairs)
        report['frames'] += 1
    report['reused_body_slots'] = dict(cpu=len(reused[0]), gpu=len(reused[1]))
    report['created_bodies'] = dict(cpu=len(lifetimes[0]), gpu=len(lifetimes[1]))
    report['identity_status'] = 'pass'
    report['residual_screen_status'] = 'fail' if any(report[field+'_failures'] for field in ['anchor', 'angular', 'cone_excess', 'lower_twist_excess', 'upper_twist_excess']) else 'pass'
    return report


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('cpu', type=Path); p.add_argument('gpu', type=Path)
    p.add_argument('--frames', type=int, required=True); p.add_argument('--output', type=Path, required=True)
    p.add_argument('--require-spherical', action='store_true')
    a = p.parse_args()
    result = compare(a.cpu, a.gpu, a.frames, a.require_spherical)
    a.output.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(result, indent=2))
    raise SystemExit(result['residual_screen_status'] != 'pass')
