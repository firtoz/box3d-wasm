#!/usr/bin/env bash
# Linux NVIDIA physics + display-local AMD rendering, with bounded one-step overlap.
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
export VK_DRIVER_FILES="${GPU_PHYSICS_SPLIT_ICDS:-/usr/share/vulkan/icd.d/nvidia_icd.json:/usr/share/vulkan/icd.d/radeon_icd.json}"
unset __NV_PRIME_RENDER_OFFLOAD __GLX_VENDOR_LIBRARY_NAME WAYLAND_DISPLAY WAYLAND_SOCKET
export GPU_PHYSICS_SPLIT_ADAPTER=1 GPU_PHYSICS_SPLIT_OVERLAP=1 GPU_PHYSICS_RENDER_QUEUE=0
export GPU_PHYSICS_GRAPH_MEMO="${GPU_PHYSICS_GRAPH_MEMO:-1}"
exec "$root/scripts/run-native-cache.sh" "$@"
