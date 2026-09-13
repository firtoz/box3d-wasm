#!/usr/bin/env bash
# Required functionality, including alive-but-non-touching contact IDs.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-$ROOT/artifacts/contact-api-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build oracle/build --target box3d_oracle -j4 >/dev/null
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/contact_lifetime_reference.cpp -I ../../box3d/include \
  "$CPU_LIB" -lpthread -lm -o target/release/contact_lifetime_cpu
cc -O2 -I ../../box3d/include -c c_abi/samples_stubs.c -o target/release/contact_api_stubs.o
g++ -O2 -std=c++17 c_abi/contact_lifetime_reference.cpp -I ../../box3d/include \
  -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s \
  -o target/release/contact_lifetime_gpu
python3 - "$OUT" <<'PY'
import hashlib, json, pathlib, subprocess, sys
out = pathlib.Path(sys.argv[1])
cases = []
for engine, shape in ((e, s) for e in ('cpu', 'gpu') for s in ('factories', 'compound-mesh-material', 'compound-mesh-remapped-high', 'compound-mesh-remapped-low', 'mesh-material-control', 'mesh-speculative-on', 'mesh-speculative-convex-off', 'mesh-speculative-mesh-off', 'mesh-speculative-sphere-off', 'shape-hole', 'shape-hole-repeated', 'shape-hole-compound', 'shape-hole-events', 'shape-hole-persistence', 'shape-hole-grow', 'shape-hole-zero', 'shape-hole-query', 'shape-hole-query-data', 'shape-reuse-history', 'shape-reuse', 'box', 'sphere', 'compound', 'compound-initial-gap', 'compound-unread', 'compound-queries', 'compound-overlap', 'compound-callbacks', 'compound-veto', 'compound-fat', 'compound-fat-batched', 'compound-fat-batched-callback', 'compound-fat-material', 'compound-fat-batched-zero', 'compound-fat-grow', 'compound-fat-grow-batched')):
    try:
        run = subprocess.run([f'target/release/contact_lifetime_{engine}', *([shape] if shape != 'box' else [])], capture_output=True, text=True, timeout=120)
        (out / (engine + '-' + shape + '.stdout')).write_text(run.stdout)
        (out / (engine + '-' + shape + '.stderr')).write_text(run.stderr)
        case = {'engine': engine, 'shape': shape, 'exit_code': run.returncode, 'status': 'pass' if run.returncode == 0 and ('hull-factories=pass' if shape == 'factories' else 'contact-lifetime=pass') in run.stdout else 'fail'}
    except (subprocess.TimeoutExpired, OSError) as exc:
        case = {'engine': engine, 'shape': shape, 'status': 'fail', 'error': str(exc)}
    cases.append(case)
inputs = [pathlib.Path('c_abi/contact_lifetime_reference.cpp'), pathlib.Path('scripts/check-contact-api-reference.sh'), pathlib.Path('c_abi/samples_stubs.c'), pathlib.Path('c_abi/shim.c'), pathlib.Path('build.rs')]
inputs += sorted(pathlib.Path('src').rglob('*.rs')) + sorted(pathlib.Path('shaders').rglob('*.wgsl'))
inputs += [pathlib.Path(f'target/release/contact_lifetime_{e}') for e in ('cpu', 'gpu')]
report = {'status': 'pass' if all(c['status'] == 'pass' for c in cases) else 'fail', 'cases': cases,
          'sha256': {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},
          'box3d_rev': subprocess.check_output(['git', '-C', '../../box3d', 'rev-parse', 'HEAD'], text=True).strip()}
(out / 'result.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps({'status': report['status'], 'cases': cases}, indent=2))
sys.exit(0 if report['status'] == 'pass' else 1)
PY
