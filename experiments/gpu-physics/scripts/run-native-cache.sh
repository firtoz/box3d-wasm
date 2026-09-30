#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
binary="$root/target/native-cache-build/release/gpu-physics"
if [ ! -x "$binary" ]; then
    echo 'Run ./scripts/build-native-cache.sh first.' >&2
    exit 1
fi
# Persist opaque driver pipelines; disable for cold-start controls.
case "${GPU_PHYSICS_PIPELINE_CACHE:-1}" in
    0) unset GPU_PHYSICS_PIPELINE_CACHE_DIR ;;
    1) export GPU_PHYSICS_PIPELINE_CACHE_DIR="${GPU_PHYSICS_PIPELINE_CACHE_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/box3d-gpu-physics/pipelines}" ;;
    *) echo 'GPU_PHYSICS_PIPELINE_CACHE must be 0 or 1' >&2; exit 1 ;;
esac
export WGPU_BACKEND=vulkan
export GPU_PHYSICS_SPIRV_LAYOUT_RUST=1
export GPU_PHYSICS_NATIVE_RADIX_CACHE=2 GPU_PHYSICS_NATIVE_CONTACT_CACHE=1
export GPU_PHYSICS_NATIVE_GRAPH_CACHE=1 GPU_PHYSICS_NATIVE_TAIL_CACHE=1
export GPU_PHYSICS_NATIVE_PAIR_CACHE="${GPU_PHYSICS_NATIVE_PAIR_CACHE:-0}" GPU_PHYSICS_PAIR_MATRIX=1
export GPU_PHYSICS_COMPONENT_TGS="${GPU_PHYSICS_COMPONENT_TGS:-1}" GPU_PHYSICS_GPU_CCD=1 GPU_PHYSICS_RESIDENT=1
export GPU_PHYSICS_GRAPH_SHARED=1 GPU_PHYSICS_DEMAND_POSES=1
export GPU_PHYSICS_SMALL_COMPONENT_WG="${GPU_PHYSICS_SMALL_COMPONENT_WG:-16}"
export GPU_PHYSICS_AB="${GPU_PHYSICS_AB:-bounded-static-sort}"
export GPU_PHYSICS_GRAPH_MEMO="${GPU_PHYSICS_GRAPH_MEMO:-0}"
cd "$root"
exec "$binary" "$@"
