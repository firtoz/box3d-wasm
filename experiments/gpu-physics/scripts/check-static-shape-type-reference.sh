#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/static-shape-type-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/static_shape_type_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/static_shape_type_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import sys,json,hashlib
out=Path(sys.argv[1])
cpu=[line.split() for line in (out/'cpu.txt').read_text().splitlines()]
gpu=[line.split() for line in (out/'gpu.txt').read_text().splitlines()]
assert len(cpu)==len(gpu)==20
for row,(a,b) in enumerate(zip(cpu,gpu)):
 if a[0]=="creation":
  assert a==b and int(a[3])==(int(a[1]) and int(a[2])) and a[4]==a[2],(a,b)
  continue
 if a[0]=="lifetime":
  assert a==b and a[0]=="lifetime" and a[4:]==["1","0","0","0","0"],(a,b)
  continue
 assert a==b,(a,b)
 kind,target,enabled,actual,valid=map(int,a[:5])
 assert actual==(target if kind==2 else 0) and valid==1,(a,b)
 assert float(a[5])==1
 assert float(a[6])==(-200 if kind==2 and enabled else 0),(a,b)
report={'status':'pass','cases':12,'coverage':'compound and height-field type rejection; sphere positive controls; enabled and disabled bodies',
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/static_shape_type_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
