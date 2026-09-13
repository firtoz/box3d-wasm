#!/usr/bin/env bash
# Native window present/draw/physics timeline. Needs a display. Not a CPU-win claim.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
OUT_DIR="${1:-$ROOT/native-timeline}"
mkdir -p "$OUT_DIR"

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

cd "$REPO"
cargo build --release --manifest-path "$ROOT/Cargo.toml"
BIN="$ROOT/target/release/gpu-physics"

run_one() {
  local name="$1" scene="$2" warmup="$3" frames="$4"
  echo "=== native-timeline $name ==="
  "$BIN" --native-timeline "$OUT_DIR/$name.json" --scene "$scene" --warmup "$warmup" --frames "$frames" --metric-runs 1 --no-sleep
}

# Five GPU trials of the two representative tiny/large scenes.
for i in 1 2 3 4 5; do
  run_one "high-resistance-run$i" high-resistance 20 60
  run_one "revolute-run$i" revolute 20 60
  run_one "dominoes-run$i" dominoes 20 60
done

echo "wrote $OUT_DIR (cpu_win_validated remains false; CPU Sokol present is not this window)"
