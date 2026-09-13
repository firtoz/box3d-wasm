#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/native-contact-semantics-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/contact_count_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 -DGPU_REFERENCE c_abi/contact_count_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
for backend in cpu gpu; do
 timeout -k 5 90 "$OUT/$backend" > "$OUT/$backend.txt" 2> "$OUT/$backend.log"
done
python3 - "$OUT" <<'PY'
from pathlib import Path
import json,hashlib,sys
out=Path(sys.argv[1]);rows={}
for backend in ['cpu','gpu']:
 rows[backend]=[dict(token.split('=') for token in line.split()) for line in (out/f'{backend}.txt').read_text().splitlines()]
assert len(rows['cpu'])==len(rows['gpu'])==48
sampled=0; non_touching=0
for c,g in zip(rows['cpu'],rows['gpu']):
 for key in ['compound','sensor','near_miss','phase']:assert c[key]==g[key]
 assert c['contacts']==c['body_capacity']==g['body_capacity'],(c,g)
 assert c['touching']==g['touching'],(c,g)
 if int(c['contacts'])>0 and c['touching']=='0':non_touching+=1
 if c['phase'] in ['stepped','separated','returned']:
  assert g['known']=='1' and g['current']=='1',(c,g)
  assert g['step']=={'stepped':'1','separated':'2','returned':'3'}[c['phase']]
  assert g['non_sensor_roots']==c['contacts'],(c,g)
  sampled+=1
 if c['phase']=='teleported':assert g['current']=='0',(c,g)
 if c['phase']=='destroyed_shape':assert g['current']=='0',(c,g)
assert sampled==24 and non_touching>0
report={'status':'pass','registry_observations':48,'current_gpu_count_observations':24,'allocated_non_touching_observations':non_touching,
'coverage':['standalone and compound children','sensor exclusion','touching and separated shapes with overlapping broadphase bounds','teleport retains contacts until step','shape destruction retires immediately'],
'public_world_contact_counter_implemented':False,'runtime_cpu_fallback_added':False,'cpu_win_validated':False,
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/contact_count_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PY
