#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
cd "$ROOT"

cargo build --release --manifest-path "$ROOT/Cargo.toml"

LIBDIR="$ROOT/target/release"
if [[ ! -f "$LIBDIR/libgpu_physics.a" ]]; then
  LIBDIR="$REPO/target/release"
fi

NV=(
  env
  __NV_PRIME_RENDER_OFFLOAD=1
  __GLX_VENDOR_LIBRARY_NAME=nvidia
  VK_DRIVER_FILES=/usr/share/vulkan/icd.d/nvidia_icd.json
)

g++ -O2 -std=c++17 "$ROOT/c_abi/stack_sample.cpp" \
  -I "$REPO/box3d/include" \
  -Wl,--allow-multiple-definition \
  "$LIBDIR/libgpu_physics.a" \
  -ldl -lpthread -lm -lgcc_s \
  -o "$ROOT/target/release/stack_sample"

g++ -O2 -std=c++17 "$ROOT/c_abi/cohort_sample.cpp" \
  -I "$REPO/box3d/include" \
  -Wl,--allow-multiple-definition \
  "$LIBDIR/libgpu_physics.a" \
  -ldl -lpthread -lm -lgcc_s \
  -o "$ROOT/target/release/cohort_sample"

echo "built stack_sample and cohort_sample"

run_cmp() {
  local label="$1"
  shift
  local A B
  A=$(mktemp)
  B=$(mktemp)
  "${NV[@]}" "$@" >"$A"
  "${NV[@]}" "$@" >"$B"
  if cmp -s "$A" "$B"; then
    echo "C ABI $label GPU-vs-GPU PASS"
    rm -f "$A" "$B"
  else
    echo "C ABI $label FAIL (dumps differ)"
    diff -u "$A" "$B" || true
    rm -f "$A" "$B"
    exit 1
  fi
}

run_cmp "stack_sample" "$ROOT/target/release/stack_sample"
run_cmp "cohort mesh-queries" "$ROOT/target/release/cohort_sample" mesh-queries
run_cmp "cohort body-controls" "$ROOT/target/release/cohort_sample" body-controls
run_cmp "cohort mesh-filter" "$ROOT/target/release/cohort_sample" mesh-filter
run_cmp "cohort mesh-events" "$ROOT/target/release/cohort_sample" mesh-events
run_cmp "cohort height-queries" "$ROOT/target/release/cohort_sample" height-queries
for scene in height-flat-convex height-slope height-hole height-clockwise-backface height-transform height-seam height-filter height-events height-wave-stress mesh-box-floor mesh-convex-drop mesh-seam-grid mesh-backside mesh-transform-scale mesh-materials height-materials mesh-ccd-sphere mesh-ccd-capsule mesh-ccd-box mesh-ccd-hull mesh-ccd-rotating mesh-ccd-disabled mesh-ccd-sensor mesh-ccd-pre-solve height-ccd-sphere height-ccd-capsule height-ccd-box height-ccd-hull height-ccd-rotating height-ccd-disabled height-ccd-sensor height-ccd-pre-solve ccd-sphere ccd-capsule ccd-box ccd-hull ccd-multi-shape ccd-spinning-stick ccd-disabled ccd-bullet-static ccd-bullet-kinematic ccd-bullet-dynamic ccd-nonbullet-dynamic ccd-bullet-bullet ccd-initial-overlap ccd-sensor ccd-pre-solve ccd-custom-filter callback-custom-accept callback-custom-reject callback-custom-opt-in callback-custom-sensor callback-pre-solve queries mover-queries explosion-radial explosion-falloff explosion-mask explosion-scale explosion-static explosion-angular explosion-compound single-box box-stack filter filter-mask filter-positive-group filter-negative-group filter-runtime contact-events contact-events-destroy body-events hit-events joint-events sensor-overlap sensor-filter sensor-filter-change sensor-destroy sensor-disable sphere-stack capsule-stack cylinder-stack cylinder-stack-plain revolute revolute-lock revolute-spring revolute-limit revolute-motor wheel-axis wheel-spring wheel-limits wheel-spin wheel-steering wheel-steering-limits wheel-controls wheel-driving spherical spherical-spring spherical-cone spherical-twist spherical-motor spherical-frames motor-velocity motor-spring distance distance-spring distance-limit distance-motor parallel prismatic prismatic-spring prismatic-motor prismatic-limit weld weld-angular-soft weld-linear-soft weld-frames weld-controls gyro-box gyro-compound compound-spheres baked-compound convex-octahedron convex-cylinder kinematic-target off-center-impulse thin-box-gap; do
  run_cmp "cohort $scene" "$ROOT/target/release/cohort_sample" "$scene"
done
