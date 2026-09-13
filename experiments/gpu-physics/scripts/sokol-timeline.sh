#!/usr/bin/env bash
# Sokol application timeline: samples cpu / gpu / both.
# Five alternating CPU/GPU trials per declared workload after a single build.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/artifacts/sokol-timeline}"
mkdir -p "$OUT"
export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

WRAP=()
if [[ -z "${DISPLAY:-}" ]] && command -v xvfb-run >/dev/null; then
  WRAP=(xvfb-run -a -s "-screen 0 1280x720x24")
fi

echo "building samples_cpu / samples_gpu / samples_both before timing"
BOX3D="$(cd "$ROOT/../.." && pwd)/box3d"
cmake -S "$ROOT/native-samples" -B "$ROOT/native-samples/build-cpu" \
  -DCMAKE_BUILD_TYPE=Release -DGPU_SAMPLES=OFF -DBOTH_SAMPLES=OFF \
  -DBOX3D_DIR="${BOX3D}" -DGPU_PHYSICS_DIR="$ROOT"
cmake --build "$ROOT/native-samples/build-cpu" --target samples_cpu -j"$(nproc)"
echo "building libgpu_physics.a"
cargo build --release --manifest-path "$ROOT/Cargo.toml"
cmake -S "$ROOT/native-samples" -B "$ROOT/native-samples/build-gpu" \
  -DCMAKE_BUILD_TYPE=Release -DGPU_SAMPLES=ON -DBOTH_SAMPLES=OFF \
  -DBOX3D_DIR="${BOX3D}" -DGPU_PHYSICS_DIR="$ROOT"
cmake --build "$ROOT/native-samples/build-gpu" --target samples_gpu -j"$(nproc)"
cmake -S "$ROOT/native-samples" -B "$ROOT/native-samples/build-both" \
  -DCMAKE_BUILD_TYPE=Release -DGPU_SAMPLES=OFF -DBOTH_SAMPLES=ON \
  -DBOX3D_DIR="${BOX3D}" -DGPU_PHYSICS_DIR="$ROOT"
cmake --build "$ROOT/native-samples/build-both" --target samples_both -j"$(nproc)"

SOURCE_HASH="$( (cd "$ROOT" && find src shaders c_abi scripts native-samples -type f ! -name '*.json' | sort | xargs sha256sum) | sha256sum | awk '{print $1}' )"
GPU_BIN_HASH="$(sha256sum "$ROOT/target/release/libgpu_physics.a" | awk '{print $1}')"
CPU_BIN_HASH="$(sha256sum "$ROOT/native-samples/build-cpu/bin/samples_cpu" | awk '{print $1}')"
ORACLE_REV="$(git -C "$ROOT/../../box3d" rev-parse --short HEAD 2>/dev/null || echo missing)"
ADAPTER="${VK_DRIVER_FILES:-}"

run_bin() {
  local mode="$1" sample="$2" extra="$3" name="$4"
  echo "=== sokol $mode $name ==="
  "${WRAP[@]}" "$ROOT/scripts/run-native-samples.sh" "$mode" \
    --sample-name "$sample" \
    --bench-json "$OUT/${name}.json" \
    --warmup 12 \
    --timed 24 \
    --no-sleep \
    --workers 4 \
    $extra
  python3 - "$OUT/${name}.json" "$mode" "$sample" "$SOURCE_HASH" "$GPU_BIN_HASH" "$CPU_BIN_HASH" "$ORACLE_REV" "$ADAPTER" <<'PY'
import json,sys
p,mode,sample,src,gpu,cpu,rev,adapter=sys.argv[1:9]
try:
    d=json.load(open(p))
except Exception as e:
    raise SystemExit(f"sokol timeline missing/invalid JSON {p}: {e}")
need=["status","mode","sample","physics_p50_ms","draw_p50_ms","render_p50_ms","ui_p50_ms","commit_p50_ms","cadence_p50_ms","frames","swap_interval"]
missing=[k for k in need if k not in d]
if missing:
    raise SystemExit(f"sokol timeline {p} missing {missing}")
if d.get("status") != "ok":
    raise SystemExit(f"sokol timeline incomplete {p}: {d.get('status')}")
if d.get("mode") != mode:
    raise SystemExit(f"sokol timeline {p} mode {d.get('mode')} != {mode}")
if d.get("unpaced") and d.get("swap_interval") != 0:
    raise SystemExit(f"sokol timeline {p}: unpaced requires verified swap interval zero, got {d.get('swap_interval')}")
d["source_sha256"]=src
d["gpu_lib_sha256"]=gpu
d["cpu_bin_sha256"]=cpu
d["box3d_rev"]=rev
d["adapter"]=adapter
d["cpu_win_validated"]=False
open(p,"w").write(json.dumps(d, indent=2)+"\n")
print(p, d.get("mode"), d.get("sample"), "physics_p50", d.get("physics_p50_ms"), "render_p50", d.get("render_p50_ms"), "draw_p50", d.get("draw_p50_ms"))
PY
}

# Five alternating CPU/GPU trials for the named large/small pair, plus paced/completed/pause/both.
for trial in 1 2 3 4 5; do
  run_bin cpu "High Resistance" "--unpaced" "cpu-hr-unpaced-t${trial}"
  run_bin gpu "High Resistance" "--unpaced" "gpu-hr-unpaced-t${trial}"
  run_bin cpu "Dominoes" "--unpaced" "cpu-dominoes-unpaced-t${trial}"
  run_bin gpu "Dominoes" "--unpaced" "gpu-dominoes-unpaced-t${trial}"
done
run_bin gpu "Dominoes" "--unpaced --completed-step" gpu-dominoes-completed-step
run_bin cpu "Single Box" "--paced" cpu-single-box-paced
run_bin gpu "Single Box" "--paced" gpu-single-box-paced
run_bin both "Dominoes" "--unpaced" both-dominoes-unpaced
run_bin gpu "Revolute" "--unpaced --pause-script" gpu-revolute-pause
echo "sokol timelines in $OUT (cpu_win_validated remains false)"
