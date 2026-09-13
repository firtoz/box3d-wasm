#!/usr/bin/env bash
# Compare latest-step event semantics, including reads skipped between steps.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/artifacts/contact-event-history-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build oracle/build --target box3d_oracle -j4 >/dev/null
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/contact_event_history_reference.cpp -I ../../box3d/include \
  "$CPU_LIB" -lpthread -lm -o target/release/contact_event_history_cpu
g++ -O2 -std=c++17 c_abi/contact_event_history_reference.cpp -I ../../box3d/include \
  -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  target/release/libgpu_physics.a "$CPU_LIB" -ldl -lpthread -lm -lgcc_s \
  -o target/release/contact_event_history_gpu
python3 - "$OUT" <<'PY'
import hashlib, json, pathlib, subprocess, sys
out = pathlib.Path(sys.argv[1])
cases = []
for name, args, expected in (
    ('skipped-end', [], (0, 0)),
    ('latest-end', ['latest-end'], (0, 1)),
    ('retouch', ['retouch'], (1, 0)),
    ('unread-begin', ['unread-begin'], (0, 1)),
):
    case = {'name': name, 'engines': {}}
    for engine in ('cpu', 'gpu'):
        try:
            run = subprocess.run([f'target/release/contact_event_history_{engine}', *args],
                                 capture_output=True, text=True, timeout=120)
            (out / f'{name}-{engine}.stdout').write_text(run.stdout)
            (out / f'{name}-{engine}.stderr').write_text(run.stderr)
            observed = json.loads(run.stdout)
            valid = run.returncode == 0 and observed == dict(
                first_begin=int(name != 'unread-begin'), first_end=0, latest_begin=expected[0], latest_end=expected[1])
            result = {'exit_code': run.returncode, 'observed': observed, 'status': 'pass' if valid else 'fail'}
        except (subprocess.TimeoutExpired, ValueError, OSError) as exc:
            result = {'status': 'fail', 'error': str(exc)}
        case['engines'][engine] = result
    case['status'] = 'pass' if all(x['status'] == 'pass' for x in case['engines'].values()) else 'fail'
    cases.append(case)
inputs = [pathlib.Path('c_abi/contact_event_history_reference.cpp'), pathlib.Path('scripts/check-contact-event-history.sh')]
inputs += sorted(pathlib.Path('src').rglob('*.rs')) + sorted(pathlib.Path('shaders').rglob('*.wgsl'))
inputs += [pathlib.Path(f'target/release/contact_event_history_{e}') for e in ('cpu', 'gpu')]
report = {'status': 'pass' if all(c['status'] == 'pass' for c in cases) else 'fail', 'cases': cases,
          'sha256': {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},
          'box3d_rev': subprocess.check_output(['git', '-C', '../../box3d', 'rev-parse', 'HEAD'], text=True).strip()}
(out / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps({'status': report['status'], 'cases': cases}, indent=2))
sys.exit(0 if report['status'] == 'pass' else 1)
PY
