#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/body-damping-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/body_damping_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/body_damping_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import sys,json,hashlib
out=Path(sys.argv[1])
cpu=[line.split() for line in (out/'cpu.txt').read_text().splitlines()]
gpu=[line.split() for line in (out/'gpu.txt').read_text().splitlines()]
assert len(cpu)==len(gpu)==36
worst=0
for a,b in zip(cpu,gpu):
 assert a[:4]==b[:4],(a,b)
 assert a[9:]==b[9:]==(['2.5','4'] if a[2]=='0' else ['0.75','1.25']),(a,b)
 for x,y in zip(a[4:9],b[4:9]):
  error=abs(float(x)-float(y));assert error<1e-5,(a,b,error)
  worst=max(worst,error)
report={'status':'pass','observations':36,'max_state_error':worst,'coverage':'creation/runtime damping, static/kinematic/dynamic motion, with and without shapes',
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/body_damping_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
