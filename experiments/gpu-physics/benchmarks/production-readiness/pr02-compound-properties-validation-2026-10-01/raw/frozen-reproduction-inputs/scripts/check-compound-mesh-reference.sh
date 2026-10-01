#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/compound-mesh-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/compound_mesh_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
if [[ "${COMPOUND_QUERY_BOTH:-0}" == "1" ]]; then
 cmake -S native-samples -B native-samples/build-both -DCMAKE_BUILD_TYPE=Release \
   -DGPU_SAMPLES=ON -DBOTH_SAMPLES=ON -DBOX3D_DIR="$ROOT/../../box3d" -DGPU_PHYSICS_DIR="$ROOT"
 cmake --build native-samples/build-both --target gpu_both_api gpu_samples_api -j8
 g++ -O2 -std=c++17 -DGPU_REFERENCE_BOTH c_abi/compound_mesh_reference.cpp -I ../../box3d/include \
   -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound -Wl,--whole-archive \
   native-samples/build-both/libgpu_both_api.a native-samples/build-both/libgpu_samples_api.a \
   -Wl,--no-whole-archive target/release/libgpu_physics.a native-samples/build-both/libbox3d_cpu.a \
   "$CPU_LIB" -lGL -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
else
 g++ -O2 -std=c++17 c_abi/compound_mesh_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
fi
for mode in 0 1; do
 "$OUT/cpu" "$mode" > "$OUT/cpu-$mode.txt" 2> "$OUT/cpu-$mode.log"
 "$OUT/gpu" "$mode" > "$OUT/gpu-$mode.txt" 2> "$OUT/gpu-$mode.log"
done
python3 - "$OUT" <<'PYCODE'
from pathlib import Path
import sys,json,math,hashlib
out=Path(sys.argv[1]);worst_ray=0;worst_support=0;worst_mover=0
for mode in [0,1]:
 cpu=[l.split() for l in (out/f'cpu-{mode}.txt').read_text().splitlines()]
 gpu=[l.split() for l in (out/f'gpu-{mode}.txt').read_text().splitlines()]
 assert len(cpu)==len(gpu)==48
 for i,(a,b) in enumerate(zip(cpu,gpu)):
  assert all(math.isfinite(float(x)) for x in a[1:]+b[1:])
  if i%6==5:
   assert a[:2]==b[:2]==['support',str(i//6)]
   ca,ga=float(a[2]),float(b[2]);assert .24<ca<.26 and .24<ga<.26
   assert float(a[3])<.001 and float(b[3])<.001
   worst_support=max(worst_support,abs(ca-ga));assert abs(ca-ga)<.0003,(mode,a,b)
  elif i%6==4:
   assert a[:2]==b[:2]==['mover-cast',str(i//6)],(mode,a,b)
   ca,ga=float(a[2]),float(b[2]);assert 0.4<ca<0.5 and 0.4<ga<0.5,(mode,a,b)
   worst_mover=max(worst_mover,abs(ca-ga));assert abs(ca-ga)<1e-5,(mode,a,b)
  elif i%6 in (2,3):
   assert a[:6]==b[:6]==['query' if i%6==2 else 'refit',str(i//6),'1','1','1','1'],(mode,a,b)
   assert abs(float(a[6])-float(b[6]))<1e-5,(mode,a,b)
  else:
   assert a[:3]==b[:3]==[str(i//6),str(i%6),str(1-i%6)],(mode,a,b)
   for x,y in zip(a[3:7],b[3:7]):
    error=abs(float(x)-float(y));worst_ray=max(worst_ray,error);assert error<1e-5,(mode,a,b)
   if i%6==0:assert a[7:]==b[7:]==['1','0'],(mode,a,b)
report={'status':'pass','backend':'both' if __import__('os').environ.get('COMPOUND_QUERY_BOTH')=='1' else 'gpu','ray_observations':32,'support_cases':16,'host_query_cases':32,'steps_per_support_case':120,'maximum_ray_error':worst_ray,'maximum_support_error':worst_support,'mover_cast_cases':16,'maximum_mover_error':worst_mover,
 'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/compound_mesh_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
PYCODE
