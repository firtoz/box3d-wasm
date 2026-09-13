#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/restitution-threshold-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/restitution_threshold_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/restitution_threshold_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PYCODE'
from pathlib import Path
import sys,json,math,hashlib
out=Path(sys.argv[1]);rows={n:[list(map(float,l.split())) for l in (out/f'{n}.txt').read_text().splitlines()] for n in ['cpu','gpu']}
assert len(rows['cpu'])==len(rows['gpu'])==24
worst=0
for i,(a,b) in enumerate(zip(rows['cpu'],rows['gpu'])):
 assert len(a)==len(b)==6 and a[:3]==b[:3]==[i//12,(i//4)%3,i%4]
 threshold=[0,1,3][int(a[1])];speed=[.5,1,2,4][int(a[2])]
 assert a[3]==b[3]==threshold
 # Require an actual impact response, with an inclusive threshold boundary.
 expected=.8*speed if speed>=threshold else 0
 assert abs(a[4]-expected)<3e-6,(a,expected)
 for x,y in zip(a[4:],b[4:]):
  assert math.isfinite(x) and math.isfinite(y)
  error=abs(x-y);worst=max(worst,error)
  assert error<=3e-6,(i,a,b,error)
report={'status':'pass','rows':24,'maximum_absolute_error':worst,'coverage':['creation threshold','post-submit runtime threshold','negative setter clamps to zero','below/equal/above impacts','actual rebound velocity and position'],
 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/restitution_threshold_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PYCODE
