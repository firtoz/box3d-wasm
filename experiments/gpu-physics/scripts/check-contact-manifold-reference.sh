#!/usr/bin/env bash
# Native layouts + executed CPU/GPU manifold semantics. Public getters remain separate.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/artifacts/contact-manifold-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build oracle/build --target box3d_oracle -j4 >/dev/null
g++ -O2 -std=c++17 c_abi/contact_persistence_reference.cpp -I ../../box3d/include \
  oracle/build/box3d-build/src/libbox3d.a -lpthread -lm -o target/release/contact_persistence_cpu
python3 - "$OUT" <<'PY'
import hashlib, json, pathlib, subprocess, sys
out = pathlib.Path(sys.argv[1])
cases = []
for name, command in (
    ('cpu-layout-and-persistence', ['target/release/contact_persistence_cpu']),
    ('rust-layout-and-decoding', ['cargo', 'test', '--release', '--lib', 'api::contact_data::tests', '--', '--test-threads=1']),
    ('gpu-published-manifold', ['cargo', 'test', '--release', '--lib', 'point_persistence_survives_publication_recycling_and_zero_impulses', '--', '--test-threads=1']),
):
    with (out / (name + '.log')).open('w') as log:
        try:
            run = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=180)
            case = {'name': name, 'exit_code': run.returncode, 'status': 'pass' if run.returncode == 0 else 'fail'}
        except (subprocess.TimeoutExpired, OSError) as exc:
            case = {'name': name, 'status': 'fail', 'error': str(exc)}
    if name != 'cpu-layout-and-persistence' and case['status'] == 'pass':
        expected = 2 if name == 'rust-layout-and-decoding' else 1
        if f'test result: ok. {expected} passed;' not in (out / (name + '.log')).read_text():
            case.update(status='fail', error='required tests were not executed')
    cases.append(case)
inputs = [pathlib.Path('c_abi/contact_persistence_reference.cpp'), pathlib.Path('scripts/check-contact-manifold-reference.sh'), pathlib.Path('target/release/contact_persistence_cpu')]
inputs += sorted(pathlib.Path('src').rglob('*.rs')) + sorted(pathlib.Path('shaders').rglob('*.wgsl'))
report = {'status': 'pass' if all(c['status'] == 'pass' for c in cases) else 'fail', 'cases': cases,
          'scope': 'native layouts, decoded manifold fields, CPU/GPU zero-impulse persistence; not public contact getters',
          'sha256': {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},
          'box3d_rev': subprocess.check_output(['git', '-C', '../../box3d', 'rev-parse', 'HEAD'], text=True).strip()}
(out / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps({'status': report['status'], 'cases': cases}, indent=2))
sys.exit(0 if report['status'] == 'pass' else 1)
PY
