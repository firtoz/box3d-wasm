#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/offset-kinematic-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release
cmake --build oracle/build --target box3d -j 4
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
cc -O2 -I ../../box3d/include -c c_abi/samples_stubs.c -o "$OUT/stubs.o"
g++ -Wall -O2 -std=c++17 c_abi/offset_kinematic_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -Wall -O2 -std=c++17 c_abi/offset_kinematic_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a "$OUT/stubs.o" "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
import json,sys,math,hashlib
from pathlib import Path
out=Path(sys.argv[1]);rows={}
for engine in ['cpu','gpu']:
 data=[list(map(float,l.split())) for l in (out/f'{engine}.txt').read_text().splitlines()]
 assert len(data)==903 and all(len(r)==22 and all(map(math.isfinite,r)) for r in data)
 assert [(int(r[0]),int(r[1])) for r in data]==[(k,s) for k in range(3) for s in range(301)]
 rows[engine]=data
worst=0
for a,b in zip(rows['cpu'],rows['gpu']):
 for i,(x,y) in enumerate(zip(a[2:],b[2:])):
  # Compare mass center, origin, rotation, world center and velocities.
  tol=1e-5
  assert abs(x-y)<=tol,(a[:2],i,x,y,tol)
  worst=max(worst,abs(x-y)/tol)
report={'status':'pass','rows':903,'worst_tolerance_ratio':worst,'coverage':['static custom center','upstream Offset Kinematic trajectory','moving kinematic center changes before and after GPU steps'],
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/offset_kinematic_reference.cpp'),Path('scripts/check-offset-kinematic-reference.sh')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
