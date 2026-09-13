#!/usr/bin/env python3
"""Validate exact Village -> Bounce House phase identity and native trajectories."""
import copy
import json
import math
import sys
from pathlib import Path
from native_scene_validate import record_complete, generic_health


def phase(doc):
    assert doc.get('status') == 'ok' and not doc.get('gpu_fail'), 'incomplete/failed run'
    assert doc.get('sample') == 'Compound/Village', 'wrong initial scene'
    assert doc.get('switch_count') == 1, 'missing/extra switch'
    assert doc.get('warmup') == 0 and doc.get('timed') == doc.get('measured') == 28, 'truncated run'
    assert doc.get('health_scan') is True, 'missing health scan'
    frames = doc.get('frames', [])
    assert len(frames) == 28, 'truncated frames'
    for i, frame in enumerate(frames):
        expected = 'Compound/Village' if i < 8 else 'Continuous/Bounce House'
        assert frame.get('sample') == expected, f'frame {i}: wrong actual scene'
        assert frame.get('i') == i and frame.get('submitted_step') == (i+1 if i<8 else i-7), 'wrong phase step'
        assert frame.get('nan_count') == 0 and frame.get('exploded') is False, 'unhealthy phase'
        if i < 8:
            assert frame.get('body_count') == 1 and frame.get('joint_count') == 0, 'Village refused or altered'
            assert frame.get('bodies') == [] and frame.get('joints') == [], 'unexpected Village dynamics'
            for key in ['min_y', 'max_y', 'max_speed']:
                assert frame.get(key) == 0, 'invalid static health'
    bounce = copy.deepcopy(doc)
    bounce.update(sample='Continuous/Bounce House', frames=copy.deepcopy(frames[8:]), timed=20, measured=20, switch_count=0)
    for i, frame in enumerate(bounce['frames']): frame['i'] = i
    status, detail = record_complete(bounce, 'Continuous/Bounce House', 20)
    assert status == 'ok', detail
    status, detail = generic_health(bounce)
    assert status == 'ok', detail
    return bounce


def compare(cpu, gpu):
    assert cpu.get('mode') == 'cpu' and gpu.get('mode') == 'gpu', 'wrong backend identity'
    c, g = phase(cpu), phase(gpu)
    for key in ['worker_count', 'enable_sleep', 'unpaced', 'completed_step_mode']:
        assert c.get(key) == g.get(key), f'unmatched {key}'
    maxima = {key: 0.0 for key in ['p', 'q', 'v', 'w']}
    for cf, gf in zip(c['frames'], g['frames']):
        assert cf['body_count'] == gf['body_count'] == 3 and gf['joint_count'] == 0
        cb, gb = cf['bodies'][0], gf['bodies'][0]
        assert cb['id'] == gb['id'], 'wrong dynamic identity'
        for key in maxima:
            maxima[key] = max(maxima[key], max(abs(a-b) for a,b in zip(cb[key],gb[key])))
    limits={'p':0.01,'q':0.001,'v':0.02,'w':0.001}
    for key, value in maxima.items(): assert value <= limits[key], f'{key}: {value} > {limits[key]}'
    return {'status':'pass','max_errors':maxima,'limits':limits,'scope':'8 static Village frames and 20 post-switch Bounce House steps',
            'settled_village_support_validated':False,'native_compatibility_validated':False,'cpu_win_validated':False}

def compare_bounce(cpu, gpu):
    assert cpu.get('mode') == 'cpu' and gpu.get('mode') == 'gpu'
    for doc in [cpu, gpu]:
        assert not doc.get('gpu_fail')
        status, detail = record_complete(doc, 'Continuous/Bounce House', 120)
        assert status == 'ok', detail
        status, detail = generic_health(doc)
        assert status == 'ok', detail
    for key in ['warmup', 'worker_count', 'enable_sleep', 'unpaced', 'completed_step_mode']:
        assert cpu.get(key) == gpu.get(key), f'unmatched {key}'
    maxima = {key: 0.0 for key in ['p','q','v','w']}
    for cf, gf in zip(cpu['frames'], gpu['frames']):
        cb, gb = cf['bodies'][0], gf['bodies'][0]
        assert cb['id'] == gb['id']
        for key in maxima:
            maxima[key] = max(maxima[key], max(abs(a-b) for a,b in zip(cb[key],gb[key])))
    limits = {'p':0.01,'q':0.001,'v':0.02,'w':0.001}
    for key,value in maxima.items(): assert value <= limits[key], f'{key}: {value} > {limits[key]}'
    return {'status':'pass','steps':120,'max_errors':maxima,'limits':limits}

if __name__ == '__main__':
    if len(sys.argv)==3 and sys.argv[1]=='--structure':
        phase(json.loads(Path(sys.argv[2]).read_text()))
    else:
        out=Path(sys.argv[1])
        result=compare(json.loads((out/'cpu-switch.json').read_text()),json.loads((out/'gpu-switch.json').read_text()))
        (out/'switch-result.json').write_text(json.dumps(result,indent=2)+'\n')
        bounce = compare_bounce(json.loads((out/'cpu-bounce.json').read_text()),json.loads((out/'gpu-bounce.json').read_text()))
        (out/'bounce-result.json').write_text(json.dumps(bounce,indent=2)+'\n')
        print(json.dumps({'switch':result,'bounce':bounce},indent=2))
