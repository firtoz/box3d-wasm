#!/usr/bin/env python3
"""Decode the actual native CCD observer words; no device or physics launch."""
from pathlib import Path
import gzip
import json
import math
import struct

ROOT = Path(__file__).resolve().parent


def floating(word):
    return struct.unpack('<f', struct.pack('<I', word))[0]


def word(value):
    return struct.unpack('<I', struct.pack('<f', value))[0]


def analyze():
    stderr = (ROOT/'raw/native/stderr.log').read_text()
    records = [json.loads(line.split(' ', 1)[1]) for line in stderr.splitlines()
               if line.startswith('gpu-native-ccd-boundary ')]
    steps = {}
    for record in records:
        if record['step'] in steps:
            assert record == steps[record['step']], 'duplicate GPU record differs'
        steps[record['step']] = record
    assert sorted(steps) == list(range(221, 234))
    trace = {}
    body = None
    for line in gzip.open(ROOT/'raw/native/comparison.txt.gz', 'rt'):
        a = line.split()
        if a[0] == 'B':
            body = int(a[2])
        elif a[0] == 'F' and body == 0:
            values = list(map(float, a[4:7]+a[8:12]+a[13:16]+a[17:20]))
            assert all(map(math.isfinite, values))
            trace[int(a[1]), int(a[2])] = dict(p=values[:3], q=values[3:7], v=values[7:10], w=values[10:])
    assert len(trace) == 480
    cpu_class = {}
    for line in stderr.splitlines():
        if line.startswith('CPUclass '):
            fields = dict(token.split('=', 1) for token in line.split()[1:])
            row = {k: [float(x) for x in v.split(',')] if k in ('p','q','v','w') else float(v)
                   for k,v in fields.items()}
            cpu_class[int(row.pop('frame'))] = row
    assert sorted(cpu_class) == list(range(220, 233))
    window = []
    for step, record in sorted(steps.items()):
        frame = step-1
        w = record['words']
        assert len(w) == 32 and record['convex_ccd_present'] and record['body_slot'] == w[0] == 1
        decoded = dict(fraction=floating(w[3]), motion_m=floating(w[4]), cutoff_m=floating(w[5]),
                       start_position=list(map(floating, w[8:11])), pre_position=list(map(floating,w[12:15])),
                       post_position=list(map(floating,w[16:19])), start_quaternion=list(map(floating,w[20:24])),
                       pre_quaternion=list(map(floating,w[24:28])), post_quaternion=list(map(floating,w[28:32])))
        assert all(map(math.isfinite, [decoded['fraction'],decoded['motion_m'],decoded['cutoff_m']]
                       + sum([decoded[k] for k in decoded if isinstance(decoded[k], list)], [])))
        assert [word(x) for x in trace[frame,1]['p']] == w[16:19]
        assert [word(x) for x in trace[frame,1]['q']] == w[28:32]
        cpu = cpu_class[frame]
        assert cpu['p'] == trace[frame,0]['p'] and cpu['q'] == trace[frame,0]['q']
        start, pre, post = [decoded[k] for k in ['start_position','pre_position','post_position']]
        reconstructed = [a+decoded['fraction']*(b-a) for a,b in zip(start,pre)]
        window.append(dict(frame=frame, step=step, body_slot=1, flags=w[1], branch=w[2],
                           actual_convex_present=True, joint_count=record['joint_count'],
                           solver_mode=record['solver_mode'],diagnostic_flags=record['diagnostic_flags'],
                           raw_words_hex=[f'{x:08x}' for x in w], GPU=decoded, CPU=cpu,
                           pre_position_difference_m=math.dist(cpu['p'],pre),
                           post_position_difference_m=math.dist(cpu['p'],post),
                           velocity_difference_m_per_s=math.dist(trace[frame,0]['v'],trace[frame,1]['v']),
                           correction_vector_m=[b-a for a,b in zip(pre,post)],correction_norm_m=math.dist(pre,post),
                           reconstructed_post=reconstructed,reconstruction_residual_m=math.dist(reconstructed,post)))
    return dict(scope='Direct GPU words from the artifact observer, exact post-position/quaternion correspondence to archived printed trace at float32 precision. Exact printed prefix neutrality does not establish complete future-state equality or full dragging acceptance. Host float64 reconstruction is diagnostic arithmetic only.',
                raw_GPU_records=len(records), unique_selected_GPU_steps=len(steps), duplicate_records_identical=True,
                CPU_TOI_records=stderr.count('CPUtoi '), window=window,
                native_internal_GPU_TOI_observed=True, original_full_dragging_acceptance=False,
                PR04_complete=False, production_candidate=False, headline_performance=False)


if __name__ == '__main__':
    result=analyze()
    (ROOT/'analysis.json').write_text(json.dumps(result,indent=2)+'\n')
    impact=next(x for x in result['window'] if x['frame']==227)
    print(json.dumps(impact,indent=2))
