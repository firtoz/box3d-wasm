#!/usr/bin/env python3
"""Validate the identical-state terrain impact, including separate terrain normals."""
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import sys


def validate(root):
    for mode in ("cpu", "gpu"):
        assert "diagnostic_split_mesh=1" not in (root/(mode+".log")).read_text(), "split meshes are diagnostic only"
    case_ids = [re.findall(r"reference_case=(\d+)", (root/(mode+".log")).read_text()) for mode in ("cpu", "gpu")]
    assert all(len(cases) == 1 for cases in case_ids), "missing or repeated reference case"
    assert case_ids[0] == case_ids[1], "CPU/GPU reference case mismatch"
    assert all(case in ("233", "842") for cases in case_ids for case in cases), "unknown reference case"
    starts = [re.findall(r"reference_start=(\d+)", (root/(mode+".log")).read_text()) or ["55"] for mode in ("cpu", "gpu")]
    assert starts[0] == starts[1] and len(starts[0]) == 1, "reference start mismatch"
    start = int(starts[0][0])
    assert start in (55, 57), "unknown reference start"
    expected_steps = list(range(start, start + 46))
    def trajectory(name):
        rows = [list(map(float, line.split())) for line in (root / name).read_text().splitlines()]
        assert len(rows) == 46, f"{name}: incomplete trajectory"
        assert [r[0] for r in rows] == expected_steps, f"{name}: wrong steps"
        assert all(len(r) == 14 and all(map(math.isfinite, r)) for r in rows), f"{name}: malformed/nonfinite state"
        return rows

    def normals(name, gpu):
        step = None
        result = {}
        for line in (root / name).read_text().splitlines():
            if line.startswith('reference_step='):
                step = int(line.split('=')[1])
                result[step] = []
            if gpu:
                match = re.search(r'ContactGpu \{ a: \d+, b: \d+, color: \d+, count: (\d+), nx: ([^,]+), ny: ([^,]+), nz: ([^,]+),', line)
                if match and int(match[1]) > 0:
                    result[step].append([float(match[i]) for i in (2, 3, 4)])
            else:
                match = re.search(r'manifold n=\(([^,]+),([^,]+),([^\)]+)\) count=(\d+)', line)
                if match and int(match[4]) > 0:
                    result[step].append([float(match[i]) for i in (1, 2, 3)])
        return result

    cpu, gpu = trajectory('cpu.txt'), trajectory('gpu.txt')
    assert max(abs(a-b) for a,b in zip(cpu[0], gpu[0])) <= 1e-7, 'starting states differ'
    cn, gn = normals('cpu.log', False), normals('gpu.log', True)
    # The seed's body overlaps both non-coplanar triangles at this checkpoint.
    reference = cn.get(58, [])
    observed = gn.get(58, [])
    assert len(reference) >= 2, 'CPU fixture did not exercise multiple terrain patches'
    matched = []
    used = set()
    for n in reference:
        candidates = [i for i,m in enumerate(observed) if i not in used and sum(x*y for x,y in zip(n,m)) > 0.995]
        if candidates:
            used.add(candidates[0])
        matched.append(bool(candidates))
    def support_gap(name, rows):
        heights = {int(step):float(y) for step,y in re.findall(r'support_height step=(\d+) y=([^\s]+)', (root/name).read_text())}
        assert sorted(heights) == expected_steps, f'{name}: missing support geometry'
        assert all(map(math.isfinite, heights.values())), f'{name}: invalid support geometry'
        return min(row[2]-heights[int(row[0])] for row in rows)
    cpu_gap = support_gap('cpu.log', cpu)
    gpu_gap = support_gap('gpu.log', gpu)
    assert cpu_gap >= -0.02, 'CPU reference itself failed the geometric support check'
    min_support_delta = min(b[2]-a[2] for a,b in zip(cpu,gpu))
    tail_speed = max(math.sqrt(sum(x*x for x in r[8:11])) for r in gpu[-10:])
    checks = dict(distinct_patch_normals=all(matched), support=gpu_gap >= -0.02,
                  settled=tail_speed <= 0.1)
    return dict(status='pass' if all(checks.values()) else 'fail', reference_case=case_ids[0][0], reference_start=start, checks=checks,
                cpu_normals_at_58=reference, gpu_normals_at_58=observed,
                min_gpu_minus_cpu_y=min_support_delta, min_cpu_above_terrain=cpu_gap,
                min_gpu_above_terrain=gpu_gap, gpu_tail_max_speed=tail_speed,
                support_tolerance_m=0.02, tail_speed_limit_m_s=0.1,
                gpu_final_y=gpu[-1][2], cpu_final_y=cpu[-1][2])


def self_check():
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        rows = [[step, 0, 0.5, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0] for step in range(55,101)]
        state = ''.join(' '.join(map(str,r))+'\n' for r in rows)
        (root/'cpu.txt').write_text(state)
        (root/'gpu.txt').write_text(state)
        support_log = 'reference_case=233 diagnostic_split_mesh=0\n' + ''.join(f'support_height step={step} y=0\n' for step in range(55,101))
        (root/'cpu.log').write_text(support_log+'reference_step=58\nmanifold n=(1,0,0) count=4\nmanifold n=(0,1,0) count=4\n')
        log = (support_log+'reference_step=58\n'
               'ContactGpu { a: 0, b: 1, color: 0, count: 4, nx: 1, ny: 0, nz: 0,\n'
               'ContactGpu { a: 0, b: 1, color: 0, count: 4, nx: 0, ny: 1, nz: 0,\n')
        (root/'gpu.log').write_text(log)
        assert validate(root)['status'] == 'pass'
        for broken_log in [log.replace('reference_case=233', 'reference_case=842'), log.replace('reference_case=233 ', '')]:
            (root/'gpu.log').write_text(broken_log)
            try:
                validate(root)
            except AssertionError:
                pass
            else:
                raise AssertionError('missing/mismatched case identity passed')
        (root/'gpu.log').write_text(log.replace('nx: 0, ny: 1','nx: 1, ny: 0'))
        assert validate(root)['status'] == 'fail', 'duplicate normals passed'
        (root/'gpu.log').write_text(log)
        for broken in [state.rsplit('\n',2)[0]+'\n', state.replace('0.5','nan',1), state.replace('0.5','0.7',1)]:
            (root/'gpu.txt').write_text(broken)
            try:
                validate(root)
            except (AssertionError, ValueError):
                pass
            else:
                raise AssertionError('invalid/mismatched trajectory passed')
        rows[-1][2] = -2.0
        (root/'gpu.txt').write_text(''.join(' '.join(map(str,r))+'\n' for r in rows))
        assert validate(root)['status'] == 'fail', 'lost support passed'
    print('mesh impact validator self-check passed (synthetic validator fixtures only)')


if __name__ == '__main__':
    if sys.argv[1:] == ['--self-check']:
        self_check()
        sys.exit(0)
    root = Path(sys.argv[1])
    try:
        result = validate(root)
    except (AssertionError, ValueError, KeyError, OSError) as error:
        result = dict(status='fail', error=str(error))
    repo = Path(__file__).resolve().parents[1]
    inputs = [repo/'c_abi/mesh_impact_reference.cpp', Path(__file__), *sorted((repo/'shaders/physics').glob('*.wgsl')),
              repo/'target/release/mesh_impact_cpu', repo/'target/release/mesh_impact_gpu']
    result['sha256'] = {str(p.relative_to(repo)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs if p.exists()}
    result['box3d_rev'] = subprocess.check_output(['git','-C',str(repo/'../../box3d'),'rev-parse','HEAD'],text=True).strip()
    (root/'result.json').write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps(result,indent=2))
    sys.exit(0 if result['status'] == 'pass' else 1)
