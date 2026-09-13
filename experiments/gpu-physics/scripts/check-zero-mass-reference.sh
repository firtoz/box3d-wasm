#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/zero-mass-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/zero_mass_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/zero_mass_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
import json,math,sys,hashlib
from pathlib import Path
out=Path(sys.argv[1]);rows={}
for engine in ['cpu','gpu']:
 data=[list(map(float,line.split())) for line in (out/f'{engine}.txt').read_text().splitlines()]
 assert len(data)==124 and all(len(r)==15 and all(map(math.isfinite,r)) for r in data)
 assert [(int(r[0]),int(r[1])) for r in data]==[(k,s) for k in range(4) for s in range(31)]
 rows[engine]=data
error=max(abs(x-y) for a,b in zip(rows['cpu'],rows['gpu']) for x,y in zip(a[2:],b[2:]))
assert error<1e-4,error
report={'status':'pass','rows':124,'max_state_error':error,'tolerance':1e-4,'cases':['angular impulse and torque','off-center linear impulse','zero inertia assigned motion','zero mass revolute motor'],
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/zero_mass_reference.cpp'),Path('scripts/check-zero-mass-reference.sh')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
