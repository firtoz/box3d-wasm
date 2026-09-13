#!/usr/bin/env bash
# Match the established native qualification display path; interactive viewer is unchanged.
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
: "${DISPLAY:?Native qualification requires an X11/Xwayland DISPLAY}"
unset WAYLAND_DISPLAY WAYLAND_SOCKET
export __NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia
export GPU_PHYSICS_PRESENT_MODE=immediate
exec "$root/scripts/run-native-cache.sh" "$@"
