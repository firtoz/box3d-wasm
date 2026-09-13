#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/speed-limit-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/speed_limit_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/speed_limit_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import sys,json,math,hashlib
out=Path(sys.argv[1])
rows={name:[list(map(float,line.split())) for line in (out/f'{name}.txt').read_text().splitlines()] for name in ['cpu','gpu']}
assert len(rows['cpu'])==len(rows['gpu'])==12
worst=0
for index,(a,b) in enumerate(zip(rows['cpu'],rows['gpu'])):
 assert len(a)==len(b)==9 and a[:2]==b[:2]==[index//3,index%3]
 assert a[2]==b[2]==[9,3,17][index%3]
 for x,y in zip(a[3:],b[3:]):
  assert math.isfinite(x) and math.isfinite(y)
  error=abs(x-y);worst=max(worst,error)
  assert error<=3e-6,(index,a,b,error)
report={'status':'pass','rows':12,'maximum_absolute_error':worst,'coverage':['creation setting','runtime decrease/increase','dynamic/kinematic/zero-mass/locked motion','velocity and integrated position'],
 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/speed_limit_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
