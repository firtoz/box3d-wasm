#!/usr/bin/env bash
# Build and run the upstream Box3D C++ samples app (sokol + imgui).
#   ./scripts/run-native-samples.sh cpu
#   ./scripts/run-native-samples.sh gpu
#   ./scripts/run-native-samples.sh both   # CPU left, GPU right, same step
set -euo pipefail

MODE="${1:-cpu}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
BOX3D="$REPO/box3d"
BUILD="$ROOT/native-samples/build-${MODE}"

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

# Persist driver pipeline products across scene switches and launches.
if [[ "$MODE" != cpu && "${GPU_PHYSICS_PIPELINE_CACHE:-1}" != 0 ]]; then
  export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}"
fi

GPU_FLAG=OFF
BOTH_FLAG=OFF
TARGET=samples_cpu
if [[ "${MODE}" == "gpu" ]]; then
  GPU_FLAG=ON
  TARGET=samples_gpu
  echo "building libgpu_physics.a"
  cargo build --release --manifest-path "$ROOT/Cargo.toml"
elif [[ "${MODE}" == "both" ]]; then
  BOTH_FLAG=ON
  TARGET=samples_both
  echo "building libgpu_physics.a"
  cargo build --release --manifest-path "$ROOT/Cargo.toml"
elif [[ "${MODE}" != "cpu" ]]; then
  echo "usage: $0 cpu|gpu|both" >&2
  exit 1
fi

cmake -S "$ROOT/native-samples" -B "$BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DGPU_SAMPLES="${GPU_FLAG}" \
  -DBOTH_SAMPLES="${BOTH_FLAG}" \
  -DBOX3D_DIR="${BOX3D}" \
  -DGPU_PHYSICS_DIR="${ROOT}"

if [[ "${MODE}" == "gpu" || "${MODE}" == "both" ]]; then
  rm -f "${BUILD}/bin/${TARGET}"
fi
cmake --build "$BUILD" --target "${TARGET}" -j"$(nproc)"

BIN="$BUILD/bin/${TARGET}"
ARGS=("${@:2}")
for i in "${!ARGS[@]}"; do
  if [[ "${ARGS[$i]}" == "--bench-json" && $((i + 1)) -lt ${#ARGS[@]} ]]; then
    json="${ARGS[$((i + 1))]}"
    if [[ "${json}" != /* ]]; then
      ARGS[$((i + 1))]="$(pwd)/${json}"
    fi
  fi
done
if [[ "${#}" -gt 1 ]]; then
  echo "running ${BIN} ${ARGS[*]} (cwd ${BOX3D})"
  cd "$BOX3D"
  exec "${BIN}" "${ARGS[@]}"
fi
echo "running ${BIN} (cwd ${BOX3D}, NVIDIA GL + Vulkan)"
cd "$BOX3D"
exec "${BIN}"
