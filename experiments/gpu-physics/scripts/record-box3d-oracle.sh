#!/usr/bin/env bash
# Real Box3D C oracle: metrics + MP4s (same camera as GPU clips) into recordings/snapshots/000-box3d-cpu/
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LABEL="${1:-000-box3d-cpu}"
FRAMES="${FRAMES:-300}"
BIN="${GPU_RECORD_BIN:-${ROOT}/target/release/gpu-physics}"
ORACLE_BUILD="${ROOT}/oracle/build"
ORACLE="${GPU_RECORD_ORACLE:-${ORACLE_BUILD}/box3d_oracle}"
OUT="${ROOT}/recordings/snapshots/${LABEL}"
DUMP="${OUT}/.dumps"

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
  mixed-topology
  anchored-mechanisms
  joint-chain
)

# Limit a fixture-only change without rewriting unrelated clips.
if [[ -n "${RECORD_SCENES:-}" ]]; then
  read -r -a SCENES <<<"${RECORD_SCENES}"
fi

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

mkdir -p "${OUT}" "${DUMP}" "${ORACLE_BUILD}"

if [[ -z "${GPU_RECORD_ORACLE:-}" ]]; then
  echo "building Box3D CPU oracle"
  cmake -S "${ROOT}/oracle" -B "${ORACLE_BUILD}" -DCMAKE_BUILD_TYPE=Release
  cmake --build "${ORACLE_BUILD}" -j
else
  test -x "${ORACLE}" || { echo "GPU_RECORD_ORACLE is not executable: ${ORACLE}" >&2; exit 1; }
fi

if [[ -z "${GPU_RECORD_BIN:-}" ]]; then
  echo "building ${BIN}"
  cargo build --release --manifest-path "${ROOT}/Cargo.toml"
else
  test -x "${BIN}" || { echo "GPU_RECORD_BIN is not executable: ${BIN}" >&2; exit 1; }
fi
python3 "${ROOT}/scripts/recording-manifest.py" "${OUT}" "${BIN}" --oracle "${ORACLE}" --frames "${FRAMES}"

echo "=== Box3D CPU metrics + dumps ==="
if [[ -n "${RECORD_SCENES:-}" ]]; then
  for scene in "${SCENES[@]}"; do
    "${ORACLE}" --scene "${scene}" --frames "${FRAMES}" --dump-dir "${DUMP}" --metrics "${OUT}/metrics-${scene}.json"
  done
else
  "${ORACLE}" --frames "${FRAMES}" --dump-dir "${DUMP}" --metrics "${OUT}/metrics.json"
fi

if [[ "${SKIP_MP4:-}" != "1" ]]; then
  for scene in "${SCENES[@]}"; do
    dest="${OUT}/${scene}.mp4"
    echo "=== replay ${scene} → ${dest} ==="
    "${BIN}" --replay-mp4 "${DUMP}/${scene}.bin" --mp4 "${dest}"
  done
else
  echo "SKIP_MP4=1 — keeping existing clips in ${OUT}"
fi

rm -rf "${DUMP}"
bun "${ROOT}/scripts/refresh-compare.ts" "${ROOT}"
echo "viewer: bun run compare  # http://127.0.0.1:8766/"
echo "clips:  ${OUT}"
