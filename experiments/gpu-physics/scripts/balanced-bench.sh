#!/usr/bin/env bash
# Equal repeated CPU/GPU completed-step trials. Does not claim a CPU win.
# Alternating trials live inside `gpu-physics --bench` (unique CPU temp dirs).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
OUT_DIR="${1:-$ROOT}"
mkdir -p "$OUT_DIR"

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

cd "$REPO"
cargo build --release --manifest-path "$ROOT/Cargo.toml"
cmake -S "$ROOT/oracle" -B "$ROOT/oracle/build" >/dev/null
cmake --build "$ROOT/oracle/build" --target box3d_oracle -j >/dev/null
BIN="$ROOT/target/release/gpu-physics"

write_env() {
  local dest="$1"
  {
    echo "{"
    echo "  \"host\": $(hostname | python3 -c 'import json,sys; print(json.dumps(sys.stdin.read().strip()))'),"
    echo "  \"date\": \"$(date -Iseconds)\","
    echo "  \"gpu_physics_sha256\": \"$(sha256sum "$BIN" | awk '{print $1}')\","
    echo "  \"oracle_sha256\": \"$(sha256sum "$ROOT/oracle/build/box3d_oracle" | awk '{print $1}')\","
    echo "  \"git\": \"$(git rev-parse --short HEAD)\","
    echo "  \"nproc\": $(nproc),"
    echo "  \"loadavg\": \"$(cat /proc/loadavg 2>/dev/null || true)\","
    echo "  \"nvidia_smi\": $(nvidia-smi --query-gpu=name,driver_version,clocks.gr,clocks.mem,power.draw,temperature.gpu --format=csv,noheader 2>/dev/null | python3 -c 'import json,sys; print(json.dumps(sys.stdin.read().strip()))' || echo '""')"
    echo "}"
  } >"$dest"
}

write_env "$OUT_DIR/bench-environment.json"

run_bench() {
  local name="$1" scene="$2" warmup="$3" frames="$4" sleep_flag="$5"
  echo "=== $name scene=$scene warmup=$warmup frames=$frames $sleep_flag ==="
  write_env "$OUT_DIR/${name}-environment.json"
  # shellcheck disable=SC2086
  "$BIN" $sleep_flag --bench "$OUT_DIR/$name.json" --scene "$scene" --warmup "$warmup" --frames "$frames" --metric-runs 5
}

# Windows: step 0, 20, 200, an early sleep-enabled window, and a late sleep-enabled window (settled only if measured asleep).
run_bench bench-revolute revolute 0 20 --no-sleep
run_bench bench-high-resistance high-resistance 0 20 --no-sleep
run_bench bench-dominoes-early dominoes 0 20 --no-sleep
run_bench bench-dominoes dominoes 20 20 --no-sleep
run_bench bench-dominoes-late dominoes 200 100 --no-sleep
run_bench bench-dominoes-sleep-early dominoes 20 20 --sleep
run_bench bench-dominoes-sleep-late dominoes 200 100 --sleep
run_bench bench-mixed-stacks mixed-stacks 0 20 --no-sleep

write_env "$OUT_DIR/bench-environment-end.json"

echo "wrote benches under $OUT_DIR (cpu_win_validated remains false)"
echo "environment: $OUT_DIR/bench-environment.json"
