#!/usr/bin/env python3
"""Preserve contiguous CPU-relative screen failures as diagnostic events.

No thresholds are changed and no event is classified as physically invalid
solely from its CPU-relative error. Retain every event, including its peak.
"""
import argparse
import json
from itertools import zip_longest
from pathlib import Path
import importlib.util
from health_identity import body_key, creation_body_map, match_joints

spec = importlib.util.spec_from_file_location('rain_compare', Path(__file__).with_name('compare-rain-lifetimes.py'))
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def summarize(cpu, gpu, frames, require_spherical=False):
    active, events = {}, []
    observations = dict(anchor=0, angular=0, cone_excess=0, lower_twist_excess=0, upper_twist_excess=0)
    for c, g in zip_longest(module.records(cpu, frames), module.records(gpu, frames)):
        if c is None or g is None or c['i'] != g['i']:
            raise ValueError('mismatched frame windows')
        mapping = creation_body_map(c['bodies'], g['bodies'])
        creations = {body_key(b): b['creation'] for b in g['bodies']}
        awake = {b['creation']: b['awake'] for b in g['bodies']}
        seen, live = set(), set()
        for a, b in match_joints(c['joints'], g['joints'], mapping):
            ends = [creations[body_key(b, 'body_a')], creations[body_key(b, 'body_b')]]
            for field, value, reference in module.joint_measurements(a, b, require_spherical):
                key = (field, b['type'], *ends)
                live.add(key)
                if value <= reference + .05:
                    continue
                observations[field] += 1
                seen.add(key)
                if key not in active:
                    active[key] = dict(field=field, type=b['type'], endpoints=ends,
                        first_frame=g['i'], last_frame=g['i'], observations=0,
                        peak_gpu=-1, peak_frame=-1, cpu_at_peak=None)
                event = active[key]
                event['last_frame'] = g['i']
                event['observations'] += 1
                event['last_awake'] = [awake[x] for x in ends]
                if not any(event['last_awake']) and 'first_asleep_frame' not in event:
                    event['first_asleep_frame'] = g['i']
                if value > event['peak_gpu']:
                    event.update(peak_gpu=value, peak_frame=g['i'], cpu_at_peak=reference)
        for key in list(active):
            if key not in seen:
                event = active.pop(key)
                event['end_reason'] = 'screen_cleared' if key in live else 'joint_removed'
                events.append(event)
    for event in active.values():
        event['end_reason'] = 'capture_ended'
    events.extend(active.values())
    events.sort(key=lambda e: (e['first_frame'], e['field'], e['endpoints']))
    return dict(scope='Unchanged CPU-relative +0.05m/+0.05rad screen; events need physical diagnosis',
                frames=frames, failing_observations=observations, event_count=len(events),
                events=events)


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('cpu', type=Path); p.add_argument('gpu', type=Path)
    p.add_argument('--frames', type=int, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--require-spherical', action='store_true')
    a = p.parse_args()
    result = summarize(a.cpu, a.gpu, a.frames, a.require_spherical)
    a.output.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k != 'events'}, indent=2))
