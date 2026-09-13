#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/density-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -Wall -O2 -std=c++17 c_abi/density_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -Wall -O2 -std=c++17 c_abi/density_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
import json,sys,math,hashlib
from pathlib import Path
out=Path(sys.argv[1]);rows={}
for engine in ['cpu','gpu']:
 data=[list(map(float,l.split())) for l in (out/f'{engine}.txt').read_text().splitlines()]
 assert len(data)==30 and all(len(r)==22 and all(map(math.isfinite,r)) for r in data)
 assert [(int(r[0]),int(r[1])) for r in data]==[(k,s) for k in range(5) for s in range(6)]
 rows[engine]=data
worst=0
for a,b in zip(rows['cpu'],rows['gpu']):
 for i,(x,y) in enumerate(zip(a[2:],b[2:])):
  # Density/mass/tensor entries scale with units; pose and velocity are absolute.
  tol=2e-6*max(1,abs(x)) if i in [0,1,5,6,7,8,9,10] else 3e-6
  assert abs(x-y)<=tol,(a[:2],i,x,y,tol)
  worst=max(worst,abs(x-y)/tol)
report={'status':'pass','rows':30,'worst_tolerance_ratio':worst,'coverage':['sphere/capsule/offset hull','static/kinematic mass frame','initial zero density','deferred density and ApplyMassFromShapes','zero and restored density','post-step mass/COM/velocities'],
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/density_reference.cpp'),Path('scripts/check-density-reference.sh')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
