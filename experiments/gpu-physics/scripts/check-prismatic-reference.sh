#!/usr/bin/env bash
# Compare identical native fixtures against real Box3D. Requires oracle/build.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/artifacts/prismatic-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
# Remove old results before compilation so a failed rebuild cannot look green.
rm -f "$OUT/result.json" "$OUT/cpu.txt" "$OUT/gpu.txt"
cargo build --release
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/prismatic_reference.cpp -I ../../box3d/include \
  "$CPU_LIB" -lpthread -lm -o target/release/prismatic_cpu
g++ -O2 -std=c++17 -DGPU_REFERENCE c_abi/prismatic_reference.cpp -I ../../box3d/include \
  -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  target/release/libgpu_physics.a "$CPU_LIB" -ldl -lpthread -lm -lgcc_s \
  -o target/release/prismatic_gpu
target/release/prismatic_cpu > "$OUT/cpu.txt"
target/release/prismatic_gpu > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
import hashlib, json, math, pathlib, subprocess, sys
out = pathlib.Path(sys.argv[1])
expected = [(mode, step, body) for mode in range(5) for step in range(1, 121) for body in range(2)]
def read(name):
    rows = [list(map(float, line.split())) for line in (out / name).read_text().splitlines()]
    assert len(rows) == len(expected), (name, 'incomplete rows', len(rows))
    for row, identity in zip(rows, expected):
        assert len(row) == 16 and tuple(row[:3]) == identity, (name, 'identity/width', row)
        assert all(math.isfinite(v) for v in row), (name, 'nonfinite', row)
    return rows
cpu, gpu = read('cpu.txt'), read('gpu.txt')
cases = []
for mode, name in enumerate(('zero-error-velocity', 'moving-line-offset-anchors', 'motor', 'spring', 'limits')):
    rows = [(a,b) for a,b in zip(cpu,gpu) if a[0] == mode]
    # q and -q encode the same orientation.
    for a,b in rows:
        if sum(x*y for x,y in zip(a[6:10],b[6:10])) < 0:
            b[6:10] = [-v for v in b[6:10]]
    errors = {key:max(abs(a[i]-b[i]) for a,b in rows for i in indices)
              for key,indices in [('position',range(3,6)),('quaternion',range(6,10)),
                                  ('velocity',range(10,13)),('angular_velocity',range(13,16))]}
    cases.append(dict(name=name, max_abs_error=errors, pass_check=max(errors.values()) <= 1e-4))
files = ['c_abi/prismatic_reference.cpp','shaders/physics/solve.wgsl','target/release/prismatic_cpu','target/release/prismatic_gpu']
result = dict(status='pass' if all(c['pass_check'] for c in cases) else 'fail',
              tolerance=1e-4, steps_per_case=120, cases=cases,
              box3d_rev=subprocess.check_output(['git','-C','../../box3d','rev-parse','HEAD'],text=True).strip(),
              sha256={f:hashlib.sha256(pathlib.Path(f).read_bytes()).hexdigest() for f in files})
(out/'result.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))
assert result['status'] == 'pass', 'prismatic CPU trajectory comparison failed'
PY
