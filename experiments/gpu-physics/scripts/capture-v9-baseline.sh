#!/usr/bin/env bash
# Freeze dirty-tree identity, run the correctness gate, then five matched CPU/GPU trials.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
STAMP="${2:-v9-baseline}"
OUT="${1:-$ROOT/artifacts/$STAMP}"
mkdir -p "$OUT"

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

cd "$REPO"
cargo build --release --manifest-path "$ROOT/Cargo.toml"
BIN="$ROOT/target/release/gpu-physics"

{
  echo "{"
  echo "  \"label\": $(python3 -c 'import json,sys; print(json.dumps(sys.argv[1]))' "$STAMP"),"
  echo "  \"date\": \"$(date -Iseconds)\","
  echo "  \"host\": $(hostname | python3 -c 'import json,sys; print(json.dumps(sys.stdin.read().strip()))'),"
  echo "  \"git\": \"$(git rev-parse HEAD)\","
  echo "  \"git_short\": \"$(git rev-parse --short HEAD)\","
  echo "  \"dirty\": $(git status --porcelain | grep -q . && echo true || echo false),"
  echo "  \"box3d_rev\": \"$(git -C "$REPO/box3d" rev-parse HEAD)\","
  echo "  \"source_tree_sha256\": \"$( (cd "$ROOT" && find src shaders c_abi scripts build.rs Cargo.toml Cargo.lock oracle/oracle.cpp native-samples -type f ! -name '*.json' ! -path '*/build-*/*' ! -path '*/target/*' ! -path '*/.fetchcontent-cache/*' 2>/dev/null | sort | xargs sha256sum | sha256sum | awk '{print $1}') )\","
  echo "  \"gpu_physics_sha256\": \"$(sha256sum "$BIN" | awk '{print $1}')\","
  echo "  \"nproc\": $(nproc),"
  echo "  \"worker_note\": \"Box3D samples clamp workerCount to cores/2 in 1..8; completed-step CPU oracle uses the oracle binary\","
  echo "  \"resolution\": \"1920x1080 Sokol default; Rust window 1280x720\","
  echo "  \"presentation\": \"Sokol swap_interval=0; paced uses LimitFrameRate; unpaced skips limiter\","
  echo "  \"commands\": ["
  echo "    \"scripts/correctness-gate.sh $OUT/correctness.json\","
  echo "    \"scripts/balanced-bench.sh $OUT\""
  echo "  ],"
  echo "  \"loadavg\": \"$(cat /proc/loadavg)\","
  echo "  \"nvidia_smi\": $(nvidia-smi --query-gpu=name,driver_version,clocks.gr,clocks.mem,power.draw,temperature.gpu --format=csv,noheader 2>/dev/null | python3 -c 'import json,sys; print(json.dumps(sys.stdin.read().strip()))' || echo '""')"
  echo "}"
} >"$OUT/freeze.json"

echo "=== correctness gate -> $OUT/correctness.json ==="
"$ROOT/scripts/correctness-gate.sh" "$OUT/correctness.json"
cp -f "$ROOT/correctness-fingerprint-inputs.txt" "$OUT/correctness-fingerprint-inputs.txt" || true

echo "=== balanced bench -> $OUT ==="
"$ROOT/scripts/balanced-bench.sh" "$OUT"

echo "baseline artifacts in $OUT"
