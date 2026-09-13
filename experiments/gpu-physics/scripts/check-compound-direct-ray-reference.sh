#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="${1:-artifacts/compound-direct-ray-reference-gate}"
mkdir -p "$OUT"
OUT="$(cd "$OUT" && pwd)"
rm -f "$OUT/result.json"
cargo build --release --lib
CPU_LIB=oracle/build/box3d-build/src/libbox3d.a
g++ -O2 -std=c++17 c_abi/compound_direct_ray_reference.cpp -I ../../box3d/include "$CPU_LIB" -lpthread -lm -o "$OUT/cpu"
g++ -O2 -std=c++17 c_abi/compound_direct_ray_reference.cpp -I ../../box3d/include -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound target/release/libgpu_physics.a target/release/contact_api_stubs.o "$CPU_LIB" -ldl -lpthread -lm -lgcc_s -o "$OUT/gpu"
"$OUT/cpu" > "$OUT/cpu.txt" 2> "$OUT/cpu.log"
"$OUT/gpu" > "$OUT/gpu.txt" 2> "$OUT/gpu.log"
python3 - "$OUT" <<'PY'
from pathlib import Path
import sys,json,hashlib,math
out=Path(sys.argv[1])
a=(out/'cpu.txt').read_text().splitlines();b=(out/'gpu.txt').read_text().splitlines()
assert len(a)==len(b)==108,(len(a),len(b))
errors=[];rays=0;scopes=0;max_error=0
for native,gpu in zip(a,b):
    if native.startswith('scope'):
        scopes+=1
        n=native.split();g=gpu.split()
        assert n==g,(n,g)
        assert int(n[3])==(0 if int(n[2])==2 else 1),n
        assert float(n[4])==1000,n
        continue
    n=list(map(float,native.split()));g=list(map(float,gpu.split()));rays+=1
    assert len(n)==len(g)==14
    assert all(math.isfinite(v) for v in n+g),(n,g)
    if n[:4]!=g[:4]:errors.append({'cpu':n,'gpu':g,'reason':'identity/hit'});continue
    if not n[3]:continue  # Native miss point/metadata are not hit information.
    error=max(abs(n[i]-g[i]) for i in [4,8,9,10,11,12,13]);max_error=max(max_error,error)
    if n[5:8]!=g[5:8] or error>1e-5:errors.append({'cpu':n,'gpu':g,'error':error})
assert rays==99 and scopes==9,(rays,scopes)
report={'status':'fail' if errors else 'pass','rays':rays,'scope_and_density_checks':scopes,'max_hit_state_error':max_error,'errors':errors,
'coverage':['public compound sphere and two-material mesh children','standalone sphere control','teleport/rotation','disabled direct query and excluded world query','initial overlap','zero translation','surface entry/exit','finite segment miss','child/triangle/compound material indices'],
'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [out/'cpu',out/'gpu',Path('c_abi/compound_direct_ray_reference.cpp')]}}
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2));assert not errors,errors
PY
