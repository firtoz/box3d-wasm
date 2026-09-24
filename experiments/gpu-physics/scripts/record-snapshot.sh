#!/usr/bin/env bash
# Record every demo scene into recordings/snapshots/<label>/ and refresh compare/snapshots.js
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LABEL="${1:-$(date +%Y-%m-%d)-baseline}"
FRAMES="${FRAMES:-300}"
METRIC_RUNS="${METRIC_RUNS:-5}"
# Pin compare-grid label (v0.5, …). Alphabetical folder names must not reshuffle versions.
if [[ -n "${VERSION:-}" ]]; then
  export GPU_COMPARE_VERSION="${VERSION}"
fi
BIN="${GPU_RECORD_BIN:-${ROOT}/target/release/gpu-physics}"
OUT="${ROOT}/recordings/snapshots/${LABEL}"

SCENES=(
  single-box
  box-stack
  sphere-stack
  capsule-stack
  revolute
  weld
  stack
  pyramid
  bounce
  mixed
  spinner
  ramp
  spheres
  dominoes
  high-resistance
  mixed-stacks
  falling-cubes
  anchored-mechanisms
  joint-chain
)

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

mkdir -p "${OUT}"
if [[ -z "${GPU_RECORD_BIN:-}" ]]; then
  echo "building ${BIN}"
  cargo build --release --manifest-path "${ROOT}/Cargo.toml"
else
  test -x "${BIN}" || { echo "GPU_RECORD_BIN is not executable: ${BIN}" >&2; exit 1; }
fi
python3 "${ROOT}/scripts/recording-manifest.py" "${OUT}" "${BIN}" --frames "${FRAMES}" --metric-runs "${METRIC_RUNS}"

if [[ "${SKIP_MP4:-}" != "1" ]]; then
  for scene in "${SCENES[@]}"; do
    dest="${OUT}/${scene}.mp4"
    echo "=== ${scene} → ${dest} (${FRAMES} frames) ==="
    "${BIN}" --scene "${scene}" --mp4 "${dest}" --frames "${FRAMES}"
  done
else
  echo "SKIP_MP4=1 — keeping existing clips in ${OUT}"
fi

echo "=== metrics → ${OUT}/metrics.json ==="
"${BIN}" --metrics "${OUT}/metrics.json" --frames "${FRAMES}" --metric-runs "${METRIC_RUNS}"

bun "${ROOT}/scripts/refresh-compare.ts" "${ROOT}"

echo "viewer: bun run compare  # http://127.0.0.1:8766/"
echo "clips:  ${OUT}"
