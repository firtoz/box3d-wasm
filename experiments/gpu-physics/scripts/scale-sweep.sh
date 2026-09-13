#!/usr/bin/env bash
# Matched GPU scale sweep. --bodies meaning is scene-specific:
#   spheres: dynamic sphere count
#   mixed-stacks: dynamic box count (plus two hidden grounds)
#   anchored-mechanisms: independent pendulums on one static ground
#   joint-chain: connected revolute links (one serial component)
#   dominoes: ring count (not body count)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
OUT="${1:-$ROOT/artifacts/scale-sweep}"
mkdir -p "$OUT"
export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"
cd "$REPO"
cargo build --release --manifest-path "$ROOT/Cargo.toml"
cmake -S "$ROOT/oracle" -B "$ROOT/oracle/build" >/dev/null
cmake --build "$ROOT/oracle/build" --target box3d_oracle -j >/dev/null
BIN="$ROOT/target/release/gpu-physics"
run() {
  local name="$1" scene="$2" bodies="$3" extra="${4:-}"
  echo "=== $name scene=$scene bodies=$bodies ($extra) ==="
  if [[ -n "$extra" ]]; then
    export GPU_PHYSICS_AB="$extra"
  else
    unset GPU_PHYSICS_AB || true
  fi
  "$BIN" --no-sleep --bench "$OUT/$name.json" --scene "$scene" --bodies "$bodies" --warmup 0 --frames 20 --metric-runs 3
  python3 - "$OUT/$name.json" "$scene" "$bodies" <<'PY'
import json,sys
p,scene,bodies=sys.argv[1:4]
d=json.load(open(p))
if d.get("scene")!=scene:
    raise SystemExit(f"{p} scene {d.get('scene')} != {scene}")
got=int(d.get("bodies",0))
need=int(bodies)
# scenes.json bodies is realized world body count (includes grounds)
print(p, "realized_bodies", got, "arg", need, "joint_dispatches last run", d.get("raw_runs",[{}])[0].get("joint_dispatches"))
PY
}
run spheres-64 spheres 64
run spheres-256 spheres 256
run spheres-1024 spheres 1024
run mixed-stacks-64 mixed-stacks 64
run mixed-stacks-256 mixed-stacks 256
run mixed-stacks-600 mixed-stacks 600
run anchored-16 anchored-mechanisms 16 serial-joints
run anchored-64 anchored-mechanisms 64 serial-joints
run anchored-256 anchored-mechanisms 256 serial-joints
run anchored-16-par anchored-mechanisms 16 parallel-joints
run anchored-64-par anchored-mechanisms 64 parallel-joints
run anchored-256-par anchored-mechanisms 256 parallel-joints
run chain-16 joint-chain 16 parallel-joints
run chain-32 joint-chain 32 parallel-joints
run chain-64 joint-chain 64 parallel-joints
run dominoes-10 dominoes 10
run dominoes-20 dominoes 20
run dominoes-30 dominoes 30
echo "scale sweep in $OUT"
