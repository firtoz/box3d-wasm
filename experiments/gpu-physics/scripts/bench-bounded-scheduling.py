#!/usr/bin/env python3
"""Two frozen fixtures, alternating fresh processes; no scaling sweep.

Build first. Use `baseline` for global/component controls and `candidate` for
alternating baseline-global/candidate-auto (requires auto support in collector).
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = [('falling-cubes', 50000), ('mixed-stacks', 4096)]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('output', type=Path)
    p.add_argument('--binary', type=Path, required=True)
    p.add_argument('--candidate-binary', type=Path)
    p.add_argument('--pilot', action='store_true', help='One candidate-only trial on each fixture')
    p.add_argument('--cpu', action='store_true')
    p.add_argument('--trials', type=int, default=3)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    binary = a.binary.resolve()
    receipt = {'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
               'fixtures': FIXTURES, 'trials': a.trials,
               'candidate_sha256': hashlib.sha256(a.candidate_binary.read_bytes()).hexdigest() if a.candidate_binary else None,
               'pilot': a.pilot, 'warmup': 90, 'timed': 240, 'substeps': 4, 'sleep': False,
               'backend': 'native', 'adapter': 'nvidia', 'color_prefix': '20'}
    receipt_path = a.output/'protocol.json'
    if receipt_path.exists():
        previous = json.loads(receipt_path.read_text())
        previous.setdefault('candidate_sha256', None)
        previous.setdefault('pilot', False)
        assert previous == json.loads(json.dumps(receipt)), 'protocol changed'
    else:
        receipt_path.write_text(json.dumps(receipt, indent=2)+'\n')
    for trial in range(1, a.trials+1):
        for scene, count in FIXTURES:
            schedules = ['global', 'auto'] if a.candidate_binary else ['global', 'component']
            if trial % 2 == 0: schedules.reverse()
            if a.pilot:
                assert a.candidate_binary and a.trials == 1
                schedules = ['auto']
            if a.cpu:
                schedules += ['cpu']
            for schedule in schedules:
                out = a.output/f'{scene}-{schedule}-{trial}'
                if (out/'trials.json').exists():
                    rows = json.loads((out/'trials.json').read_text())
                    assert len(rows) == 1 and rows[0]['status'] == 'ok', 'incomplete trial; inspect before resume'
                    continue
                cmd = [sys.executable, str(ROOT/'scripts/bench-falling-cubes.py'), str(out),
                       '--scene', scene, '--counts', str(count), '--modes', 'physics-cpu' if schedule=='cpu' else 'physics-gpu',
                       '--trials', '1', '--warmup', '90', '--timed', '240', '--min-rate', '0',
                       '--selected-binaries-only', '--gpu-binary', str(a.candidate_binary.resolve() if schedule=='auto' else binary),
                       '--gpu-solver', 'global' if schedule=='cpu' else schedule, '--gpu-color-prefix', '20']
                print(f'trial {trial}: {scene} {schedule}', flush=True)
                subprocess.run(cmd, cwd=ROOT, check=True)

if __name__ == '__main__':
    main()
