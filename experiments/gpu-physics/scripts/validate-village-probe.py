#!/usr/bin/env python3
"""Validate the named dynamic interaction, not general Village compatibility."""
import hashlib
import json
import math
import re
from pathlib import Path
import sys

out = Path(sys.argv[1])
reports = {mode: json.loads((out / f'{mode}-drop.json').read_text()) for mode in ['cpu', 'gpu']}
worst = 0.0
for mode, report in reports.items():
    assert report['mode'] == mode and report['sample'] == 'Compound/Village'
    assert report['status'] == 'ok' and report['measured'] == 240 and report['timed'] == 240
    assert not report['gpu_fail'] and len(report['frames']) == 240
    assert 'village-drop body=2 ' in (out / f'{mode}-drop.log').read_text()
    for i, frame in enumerate(report['frames']):
        assert frame['submitted_step'] == i + 1 and frame['body_count'] == 2
        assert not frame['nan_count'] and not frame['exploded']
        assert len(frame['bodies']) == 1 and frame['bodies'][0]['id'] == 2
for cpu, gpu in zip(reports['cpu']['frames'], reports['gpu']['frames']):
    for field in ['p', 'q', 'v', 'w']:
        for a, b in zip(cpu['bodies'][0][field], gpu['bodies'][0][field]):
            assert math.isfinite(a) and math.isfinite(b)
            worst = max(worst, abs(a - b))
assert worst <= 1e-5, f'drop trajectory mismatch: {worst}'
log = (out / 'gpu-drop.log').read_text()
assert 'shapes=52502 ' in log and 'mesh_vertices=2678 mesh_triangles=4370 mesh_nodes=1571 ' in log
result = {
    'status': 'pass', 'scope': 'unchanged Village plus scripted sphere drop',
    'ordinary_compound_import': True,
    'steps': 240, 'maximum_recorded_state_error': worst, 'epsilon': 1e-5,
    'gpu_scene_heap_bytes': int(re.findall(r'scene_heap_bytes=(\d+)', log)[-1]),
    'gpu_contact_counter_validated': False,
    'gpu_contact_counter_note': 'C GetCounters currently leaves contactCount zero; do not interpret it as measured contacts',
    'settled_support_validated': False, 'native_compatibility': False,
    'cpu_win_validated': False, 'trials': 1,
    'cadence_p50_ms': {m: r['cadence_p50_ms'] for m, r in reports.items()},
    'physics_p50_ms': {m: r['physics_p50_ms'] for m, r in reports.items()},
    'sha256': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in out.glob('*-drop.json')}
}
(out / 'drop-result.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result))
