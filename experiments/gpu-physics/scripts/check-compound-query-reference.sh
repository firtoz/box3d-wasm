#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/compound-query-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/compound_query_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/compound_query_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import sys,json,hashlib
out=Path(sys.argv[1])
cpu=(out/'cpu.txt').read_text().split();gpu=(out/'gpu.txt').read_text().split()
expected=['1','1','1','1','2','1','1','1','ray-left','1','0.25','0','100','ray-right','1','0.25','1','101','1']
assert cpu==gpu==expected,(cpu,gpu,expected)
report={'status':'pass','observations':['one overlap callback','one AABB callback','one unclipped ray callback','one mover callback','two mover planes','one shape-cast callback','one mover-filter callback','public shape identity'],
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/compound_query_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
