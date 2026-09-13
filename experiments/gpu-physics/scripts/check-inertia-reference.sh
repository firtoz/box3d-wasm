#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/inertia-reference}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/inertia_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/inertia_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
python3 - "$OUT" <<'PY'
import json,math,hashlib,subprocess,sys
from pathlib import Path
out=Path(sys.argv[1]);traces={}
for engine in ['cpu','gpu']:
 with (out/f'{engine}.txt').open('w') as stdout,(out/f'{engine}.log').open('w') as stderr:
  subprocess.run([str(out/engine)],stdout=stdout,stderr=stderr,timeout=120,check=True)
 assert 'Validation Error' not in (out/f'{engine}.log').read_text()
 trace={}
 for line in (out/f'{engine}.txt').read_text().splitlines():
  words=line.split();assert len(words)==20
  key=tuple(map(int,words[:2]));values=list(map(float,words[2:]));assert all(map(math.isfinite,values))
  assert key not in trace;trace[key]=values
 assert set(trace)=={(k,p) for k in range(6) for p in range(4)}
 traces[engine]=trace
errors=[]
for key in traces['cpu']:
 a,b=traces['cpu'][key],traces['gpu'][key]
 # Native setup uses float math; preserve a relative tolerance for large tensors
 # and an absolute floor for inverse-tensor entries close to zero.
 for x,y in zip(a,b):assert abs(x-y)<=max(1e-6,2e-6*abs(x)),(key,x,y)
 errors.append({'kind':key[0],'phase':key[1],'max_error':max(abs(x-y) for x,y in zip(a,b))})
inputs=[Path('c_abi/inertia_reference.cpp'),Path('c_abi/shim.c'),Path('c_abi/both_dual.c'),Path('src/api/world.rs'),Path('src/c_abi.rs'),Path('scripts/check-inertia-reference.sh'),out/'cpu',out/'gpu']
report={'status':'pass','comparisons':24,'errors':errors,'limitations':['Getter parity; does not establish identical solver inertia or solve response.'], 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print('inertia-reference: 24 comparisons pass')
PY
