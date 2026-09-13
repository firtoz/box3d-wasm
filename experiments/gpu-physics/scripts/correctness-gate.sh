#!/usr/bin/env bash
# Executable correctness gate. Nonzero exit if any required check fails.
# Missing long-run or CPU evidence is recorded as skipped, not pass.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
if [[ "${1:-}" == /* ]]; then
  OUT="$1"
elif [[ -n "${1:-}" ]]; then
  OUT="$ROOT/$1"
else
  OUT="$ROOT/correctness-latest.json"
fi
BIN="$ROOT/target/release/gpu-physics"
CASES="$ROOT/correctness-cases.jsonl"

export __NV_PRIME_RENDER_OFFLOAD="${__NV_PRIME_RENDER_OFFLOAD:-1}"
export __GLX_VENDOR_LIBRARY_NAME="${__GLX_VENDOR_LIBRARY_NAME:-nvidia}"
export VK_DRIVER_FILES="${VK_DRIVER_FILES:-/usr/share/vulkan/icd.d/nvidia_icd.json}"

cd "$REPO"
write_fingerprint_inputs() {
  local dest="$1"
  {
    echo "=== files ==="
    (cd "$ROOT" && find src shaders c_abi scripts build.rs Cargo.toml Cargo.lock oracle/oracle.cpp native-samples \
      -type f ! -name '*.json' ! -name '*.mp4' \
      ! -path '*/build-*/*' ! -path '*/target/*' ! -path '*/.fetchcontent-cache/*' \
      2>/dev/null | sort | xargs sha256sum)
    echo "=== gpu-physics bin ==="
    sha256sum "$BIN" 2>/dev/null || true
    echo "=== oracle bin ==="
    sha256sum "$ROOT/oracle/build/box3d_oracle" 2>/dev/null || true
    echo "=== rustc ==="
    rustc --version --verbose
    echo "=== adapter ==="
    echo "adapter=${VK_DRIVER_FILES:-}"
    echo "profile=release fused_islands=false"
    vulkaninfo --summary 2>/dev/null | head -n 40 || true
    nvidia-smi -L 2>/dev/null || true
    nvidia-smi --query-gpu=name,driver_version --format=csv 2>/dev/null || true
  } >"$dest"
}
source_fingerprint() {
  write_fingerprint_inputs "$ROOT/correctness-fingerprint-inputs.txt"
  sha256sum "$ROOT/correctness-fingerprint-inputs.txt" | awk '{print $1}'
}
BOX3D_REV="$(git -C "$REPO/box3d" rev-parse --short HEAD 2>/dev/null || echo missing)"
GIT="$(git rev-parse --short HEAD)"
if git status --porcelain | grep -q .; then DIRTY=true; else DIRTY=false; fi

: >"$CASES"
pass=0
fail=0
skip=0

record() {
  local name="$1" status="$2" detail="$3"
  printf '{"name":"%s","status":"%s","detail":"%s"}\n' "$name" "$status" "$detail" >>"$CASES"
  case "$status" in
    pass) pass=$((pass + 1)) ;;
    fail) fail=$((fail + 1)) ;;
    skip) skip=$((skip + 1)) ;;
  esac
}

echo "building release gpu-physics"
cargo build --release --manifest-path "$ROOT/Cargo.toml"

echo "rebuild Box3D oracle"
cmake -S "$ROOT/oracle" -B "$ROOT/oracle/build" >/dev/null
cmake --build "$ROOT/oracle/build" --target box3d_oracle -j >/dev/null
ORACLE="$ROOT/oracle/build/box3d_oracle"
if [[ ! -x "$ORACLE" ]]; then
  record "cpu_oracle_rebuild" "fail" "oracle executable missing after rebuild"
else
  record "cpu_oracle_rebuild" "pass" "rebuilt box3d_oracle"
fi

SOURCE_HASH="$(source_fingerprint)"

echo "cargo test --lib -- --test-threads=1"
if cargo test --release --lib --manifest-path "$ROOT/Cargo.toml" -- --test-threads=1; then
  record "cargo_test_lib" "pass" "gpu_invariants + lib tests"
else
  record "cargo_test_lib" "fail" "see cargo output"
fi

run_self() {
  local scene="$1" frames="$2" extra="${3:-}"
  local name="self-test:${scene}:${frames}${extra:+:$extra}"
  # shellcheck disable=SC2086
  if "$BIN" --self-test --scene "$scene" --frames "$frames" $extra; then
    record "$name" "pass" "GPU-vs-GPU dumps identical"
  else
    record "$name" "fail" "dumps differ or process error"
  fi
}

if "$ROOT/scripts/check-prismatic-reference.sh" "$ROOT/artifacts/prismatic-reference-gate"; then
  record "cpu_oracle:prismatic" "pass" "five native fixtures, 120 steps, all pose/velocity components within 1e-4"
else
  record "cpu_oracle:prismatic" "fail" "missing/incomplete/incorrect native prismatic trajectory"
fi

if "$ROOT/scripts/check-mesh-impact-reference.sh" "$ROOT/artifacts/mesh-impact-reference-gate"; then
  record "cpu_oracle:mesh-impact" "pass" "shared-state Mesh Drop impact: separate normals, support, settled velocity"
else
  record "cpu_oracle:mesh-impact" "fail" "missing/incorrect terrain manifolds or lost support in shared-state impact"
fi

if "$ROOT/scripts/check-mesh-impact-reference.sh" "$ROOT/artifacts/mesh-impact-842-reference-gate" 842; then
  record "cpu_oracle:mesh-impact-842" "pass" "shared-state body 842: all CPU patch normals, support, settled velocity"
else
  record "cpu_oracle:mesh-impact-842" "fail" "missing/incorrect terrain patches or lost support in body 842 impact"
fi


if "$ROOT/scripts/check-contact-event-history.sh" "$ROOT/artifacts/contact-event-history-gate"; then
  record "cpu_oracle:contact-event-history" "pass" "skipped end, latest end, re-touch and unread begin match latest-step CPU events"
else
  record "cpu_oracle:contact-event-history" "fail" "missing/incorrect latest-step contact events; see contact-event-history-gate/result.json"
fi

if "$ROOT/scripts/check-contact-manifold-reference.sh" "$ROOT/artifacts/contact-manifold-reference-gate"; then
  record "cpu_oracle:contact-manifold-data" "pass" "native layouts and CPU/GPU persisted manifold data, including zero impulses"
else
  record "cpu_oracle:contact-manifold-data" "fail" "missing/incorrect native layout or manifold decoding; see contact-manifold-reference-gate"
fi

if "$ROOT/scripts/check-hull-edge-reference.sh" "$ROOT/artifacts/hull-edge-reference-gate"; then
  record "cpu_oracle:hull-edge" "pass" "edge-separated hulls, edge witness, and nearby face manifold"
else
  record "cpu_oracle:hull-edge" "fail" "hull edge geometry mismatch; see hull-edge-reference-gate"
fi

if "$ROOT/scripts/check-hull-mesh-rest-reference.sh" "$ROOT/artifacts/hull-mesh-rest-reference-gate"; then
  record "cpu_oracle:hull-mesh-rest" "pass" "Gear Lift resting rock mesh normals and penetration match CPU"
else
  record "cpu_oracle:hull-mesh-rest" "fail" "mesh face/edge witness mismatch; see hull-mesh-rest-reference-gate"
fi

if "$ROOT/scripts/check-hull-capsule-witness.sh" "$ROOT/artifacts/hull-capsule-witness-gate"; then
  record "cpu_oracle:hull-capsule-witness" "pass" "captured Gear Lift pair, swapped/rotated frames, face/corner/miss and symmetric core geometry"
else
  record "cpu_oracle:hull-capsule-witness" "fail" "hull-capsule contact geometry mismatch; see hull-capsule-witness-gate"
fi

if "$ROOT/scripts/check-gear-impact-reference.sh" "$ROOT/artifacts/gear-impact-reference-gate"; then
  record "cpu_oracle:gear-impact" "pass" "shared-state first impact pose/velocity with full inertia and mesh rolling resistance"
else
  record "cpu_oracle:gear-impact" "fail" "shared-state Gear Lift impact mismatch; see gear-impact-reference-gate"
fi

if "$ROOT/scripts/check-inertia-reference.sh" "$ROOT/artifacts/inertia-reference-gate"; then
  record "cpu_oracle:inertia-getters" "pass" "local/world inertia across body types, locks, teleport, post-step and zero mass"
else
  record "cpu_oracle:inertia-getters" "fail" "native inertia getter mismatch; see inertia-reference-gate"
fi

if "$ROOT/scripts/check-contact-api-reference.sh" "$ROOT/artifacts/contact-api-reference-gate"; then
  record "cpu_oracle:contact-api-lifetime" "pass" "body/shape data, event IDs, alive non-touching contacts and ID retirement/reuse"
else
  record "cpu_oracle:contact-api-lifetime" "fail" "public contact getters or lifetime semantics are incomplete; see contact-api-reference-gate"
fi

run_self box-stack 300
run_self revolute 300
run_self weld 300
run_self dominoes 120
run_self dominoes 300
run_self high-resistance 600 --no-sleep
run_self high-resistance 600 --sleep
run_self mixed-stacks 120 --no-sleep

echo "C ABI stack_sample (setter + dump repeatability)"
STACK_SRC="$ROOT/c_abi/stack_sample.cpp"
STACK_BIN="$ROOT/target/release/stack_sample"
if g++ -O2 -std=c++17 "$STACK_SRC" -I "$REPO/box3d/include" -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  "$ROOT/target/release/libgpu_physics.a" "$ROOT/oracle/build/box3d-build/src/libbox3d.a" -ldl -lpthread -lm -lgcc_s -o "$STACK_BIN"; then
  A=$(mktemp); B=$(mktemp)
  if "$STACK_BIN" >"$A" && "$STACK_BIN" >"$B" && cmp -s "$A" "$B"; then
    record "c_abi:stack_sample" "pass" "GPU-vs-GPU dumps identical; setter check"
    rm -f "$A" "$B"
  else
    record "c_abi:stack_sample" "fail" "dumps differ or setter failed"
    rm -f "$A" "$B"
  fi
else
  record "c_abi:stack_sample" "fail" "compile failed"
fi

echo "C ABI high_resistance_sample"
HR_SRC="$ROOT/c_abi/high_resistance_sample.cpp"
HR_BIN="$ROOT/target/release/high_resistance_sample"
if g++ -O2 -std=c++17 "$HR_SRC" -I "$REPO/box3d/include" -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  "$ROOT/target/release/libgpu_physics.a" "$ROOT/oracle/build/box3d-build/src/libbox3d.a" -ldl -lpthread -lm -lgcc_s -o "$HR_BIN"; then
  if "$HR_BIN"; then
    record "c_abi:high_resistance" "pass" "native C ABI High Resistance settled"
  else
    record "c_abi:high_resistance" "fail" "settling or pause check failed"
  fi
else
  record "c_abi:high_resistance" "fail" "compile failed"
fi

echo "C ABI mixed_stacks_sample"
MIX_SRC="$ROOT/c_abi/mixed_stacks_sample.cpp"
MIX_BIN="$ROOT/target/release/mixed_stacks_sample"
if g++ -O2 -std=c++17 "$MIX_SRC" -I "$REPO/box3d/include" -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  "$ROOT/target/release/libgpu_physics.a" "$ROOT/oracle/build/box3d-build/src/libbox3d.a" -ldl -lpthread -lm -lgcc_s -o "$MIX_BIN"; then
  if "$MIX_BIN"; then
    record "c_abi:mixed_stacks" "pass" "overlapping statics, 600 boxes stayed stacked"
  else
    record "c_abi:mixed_stacks" "fail" "COM y exploded or bodies missing"
  fi
else
  record "c_abi:mixed_stacks" "fail" "compile failed"
fi

echo "C ABI compound_overlap_sample"
COMP_SRC="$ROOT/c_abi/compound_overlap_sample.cpp"
COMP_BIN="$ROOT/target/release/compound_overlap_sample"
if g++ -O2 -std=c++17 "$COMP_SRC" -I "$REPO/box3d/include" -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  "$ROOT/target/release/libgpu_physics.a" "$ROOT/oracle/build/box3d-build/src/libbox3d.a" -ldl -lpthread -lm -lgcc_s -o "$COMP_BIN"; then
  if "$COMP_BIN"; then
    record "c_abi:compound_overlap" "pass" "overlapping compound children stayed on ground"
  else
    record "c_abi:compound_overlap" "fail" "compound COM y left rest"
  fi
else
  record "c_abi:compound_overlap" "fail" "compile failed"
fi

if bash "$ROOT/scripts/check-body-damping-reference.sh" "$ROOT/artifacts/body-damping-reference-gate"; then
  record "cpu_oracle:body-damping" "pass" "creation/runtime damping and static/kinematic/dynamic motion"
else
  record "cpu_oracle:body-damping" "fail" "native body motion comparison failed"
fi

if bash "$ROOT/scripts/check-static-shape-type-reference.sh" "$ROOT/artifacts/static-shape-type-reference-gate"; then
  record "cpu_oracle:static-shape-type" "pass" "static-only geometry, enabled state and ID lifetime"
else
  record "cpu_oracle:static-shape-type" "fail" "native type/identity comparison failed"
fi

if bash "$ROOT/scripts/check-compound-import-failure.sh"; then
  record "compound_import:failure" "pass" "GPU and combined parent/child allocation and attachment refusal"
else
  record "compound_import:failure" "fail" "partial compound import was not refused"
fi

if bash "$ROOT/scripts/check-compound-mesh-cache.sh"; then
  record "compound_mesh_cache:lifetime" "pass" "shared source and allocation/source/clone failure cleanup under sanitizers"
else
  record "compound_mesh_cache:lifetime" "fail" "shared raw mesh cache control failed"
fi

if bash "$ROOT/scripts/check-compound-mesh-bake.sh"; then
  record "compound_mesh_bake:allocation" "pass" "8 orientations, 3 allocation failures, address/leak sanitizer"
else
  record "compound_mesh_bake:allocation" "fail" "shared mesh bake host control failed"
fi

if COMPOUND_QUERY_BOTH=1 bash "$ROOT/scripts/check-compound-mesh-reference.sh" artifacts/both-compound-mesh-reference-gate; then
  record "cpu_oracle:both-compound-mesh" "pass" "real dual-world bridge: mirrored mesh rays, movers and physical support"
else
  record "cpu_oracle:both-compound-mesh" "fail" "combined compound mesh reference failed"
fi

if bash "$ROOT/scripts/check-contact-count-semantics.sh"; then
  record "cpu_oracle:contact-count-semantics" "pass" "native compound-child/sensor/non-touching allocation lifecycle and scheduled GPU roots"
else
  record "cpu_oracle:contact-count-semantics" "fail" "native contact allocation lifecycle reference failed"
fi

if bash "$ROOT/scripts/check-world-counters.sh"; then
  record "cpu_oracle:world-counters" "pass" "public compound shapes and live joint holes match native topology counts"
else
  record "cpu_oracle:world-counters" "fail" "native topology counter reference failed"
fi

if bash "$ROOT/scripts/check-query-reentrancy.sh"; then
  record "cpu_oracle:query-reentrancy" "pass" "callback reads and nested queries; stop/ignore/clip semantics vs native"
else
  record "cpu_oracle:query-reentrancy" "fail" "callback ray reentrancy reference failed or timed out"
fi

if bash "$ROOT/scripts/check-compound-direct-ray-reference.sh"; then
  record "cpu_oracle:compound-direct-ray" "pass" "direct compound/disabled rays and material indices vs native"
else
  record "cpu_oracle:compound-direct-ray" "fail" "direct shape ray reference failed"
fi

if bash "$ROOT/scripts/check-compound-query-reference.sh" "$ROOT/artifacts/compound-query-reference-gate"; then
  record "cpu_oracle:compound-query" "pass" "public identity, callback cardinality, closest child and mover plane batching"
else
  record "cpu_oracle:compound-query" "fail" "compound query native comparison failed"
fi

if bash "$ROOT/scripts/check-compound-mesh-reference.sh" "$ROOT/artifacts/compound-mesh-reference-gate"; then
  record "cpu_oracle:compound-mesh" "pass" "translated/mirrored/oblique rays, compound identity and support"
else
  record "cpu_oracle:compound-mesh" "fail" "compound mesh transform native comparison failed"
fi

if bash "$ROOT/scripts/check-restitution-threshold-reference.sh" "$ROOT/artifacts/restitution-threshold-reference-gate"; then
  record "cpu_oracle:restitution-threshold" "pass" "creation/runtime settings and impact threshold boundary"
else
  record "cpu_oracle:restitution-threshold" "fail" "restitution threshold native comparison failed"
fi

if bash "$ROOT/scripts/check-speed-limit-reference.sh" "$ROOT/artifacts/speed-limit-reference-gate"; then
  record "cpu_oracle:speed-limit" "pass" "creation/runtime limit, dynamic/kinematic/zero-mass/locked integration"
else
  record "cpu_oracle:speed-limit" "fail" "speed limit native comparison failed"
fi

if bash "$ROOT/scripts/check-joint-collision-reference.sh" "$ROOT/artifacts/joint-collision-reference-gate"; then
  record "cpu_oracle:joint-collision" "pass" "toggle, multiple vetoes, immediate retirement, IDs and deferred events"
else
  record "cpu_oracle:joint-collision" "fail" "joint collision lifecycle comparison failed"
fi

if bash "$ROOT/scripts/check-joint-metadata-reference.sh" "$ROOT/artifacts/joint-metadata-reference-gate"; then
  record "cpu_oracle:joint-metadata" "pass" "nine joint types, endpoints, world IDs and body reuse/destruction"
else
  record "cpu_oracle:joint-metadata" "fail" "joint identity native comparison failed"
fi

if bash "$ROOT/scripts/check-density-reference.sh" "$ROOT/artifacts/density-reference-gate"; then
  record "cpu_oracle:density" "pass" "density mass/COM updates, zero transitions and body types"
else
  record "cpu_oracle:density" "fail" "density native comparison failed"
fi

if bash "$ROOT/scripts/check-body-dynamics-reference.sh" "$ROOT/artifacts/body-dynamics-reference-gate"; then
  record "cpu_oracle:body-dynamics" "pass" "Body Type four-point impact and 600-step gyro/custom-inertia trajectory"
else
  record "cpu_oracle:body-dynamics" "fail" "native body dynamics comparison failed"
fi

if bash "$ROOT/scripts/check-zero-mass-reference.sh" "$ROOT/artifacts/zero-mass-reference-gate"; then
  record "cpu_oracle:zero-mass" "pass" "dynamic zero-mass motion, impulses, torque and revolute motor"
else
  record "cpu_oracle:zero-mass" "fail" "zero-mass native comparison failed"
fi

echo "C ABI query cohorts (pose consistency and GPU triangle picking)"
COHORT_SRC="$ROOT/c_abi/cohort_sample.cpp"
COHORT_BIN="$ROOT/target/release/cohort_sample"
if g++ -O2 -std=c++17 "$COHORT_SRC" -I "$REPO/box3d/include" -Wl,--allow-multiple-definition -Wl,--wrap=b3CreateCompound \
  "$ROOT/target/release/libgpu_physics.a" "$ROOT/oracle/build/box3d-build/src/libbox3d.a" -ldl -lpthread -lm -lgcc_s -o "$COHORT_BIN"; then
  for cohort in queries mesh-queries height-queries; do
    if "$COHORT_BIN" "$cohort"; then
      record "c_abi:$cohort" "pass" "query assertions executed"
    else
      record "c_abi:$cohort" "fail" "query contract failed"
    fi
  done
else
  record "c_abi:queries" "fail" "compile failed"
fi

echo "native scene stability matrix (scoped separately from the 22-case physics suite)"
if bash "$ROOT/scripts/native-scene-gate.sh" "$ROOT/native-scene-latest.json"; then
  record "native_scene_matrix" "pass" "reported native samples stayed finite without abort"
else
  record "native_scene_matrix" "fail" "see native-scene-latest.json"
fi

echo "long-run box-stack 3600 (catastrophe + stack bounds)"
if "$BIN" --headless --scene box-stack --frames 3600 --no-sleep; then
  record "long-run:box-stack:3600" "pass" "catastrophe checks plus stack rest"
else
  record "long-run:box-stack:3600" "fail" "crash or physical-quality error"
fi

echo "long-run dominoes 3600 (catastrophe checks)"
if "$BIN" --headless --scene dominoes --frames 3600 --no-sleep; then
  record "long-run:dominoes:3600" "pass" "catastrophe checks"
else
  record "long-run:dominoes:3600" "fail" "crash or physical-quality error"
fi

echo "long-run high-resistance 3600 (settling quality)"
if "$BIN" --headless --scene high-resistance --frames 3600 --no-sleep; then
  record "long-run:high-resistance:3600" "pass" "settling classification and support"
else
  record "long-run:high-resistance:3600" "fail" "crash or settling-quality error"
fi

DUMP_DIR="$(mktemp -d)"
if [[ -x "$ORACLE" ]]; then
  if "$ORACLE" --scene revolute --frames 301 --dump-dir "$DUMP_DIR" --no-sleep >/dev/null; then
    if "$BIN" --compare-oracle "$DUMP_DIR/revolute.bin" --scene revolute --epsilon 0.25; then
      record "cpu_oracle_compare:revolute" "pass" "joint compare with pos/vel/quat/omega tols"
    else
      record "cpu_oracle_compare:revolute" "fail" "physical compare exceeded tolerance"
    fi
  else
    record "cpu_oracle_compare:revolute" "fail" "oracle dump failed"
  fi
  if "$ORACLE" --scene high-resistance --frames 601 --dump-dir "$DUMP_DIR" --no-sleep >/dev/null; then
    if "$BIN" --compare-oracle "$DUMP_DIR/high-resistance.bin" --scene high-resistance --epsilon 0.002 --no-sleep; then
      record "cpu_oracle_compare:high-resistance" "pass" "every-step 300..600 settling vs Box3D"
    else
      record "cpu_oracle_compare:high-resistance" "fail" "High Resistance CPU compare failed"
    fi
  else
    record "cpu_oracle_compare:high-resistance" "fail" "oracle dump failed"
  fi
  if "$ORACLE" --scene high-resistance --frames 601 --dump-dir "$DUMP_DIR" >/dev/null; then
    if "$BIN" --compare-oracle "$DUMP_DIR/high-resistance.bin" --scene high-resistance --epsilon 0.002 --sleep; then
      record "cpu_oracle_compare:high-resistance-sleep" "pass" "sleep-enabled High Resistance vs Box3D"
    else
      record "cpu_oracle_compare:high-resistance-sleep" "fail" "sleep-enabled High Resistance CPU compare failed"
    fi
  else
    record "cpu_oracle_compare:high-resistance-sleep" "fail" "oracle dump failed"
  fi
  if "$ORACLE" --scene mixed-stacks --frames 121 --dump-dir "$DUMP_DIR" --no-sleep >/dev/null; then
    if "$BIN" --compare-oracle "$DUMP_DIR/mixed-stacks.bin" --scene mixed-stacks --epsilon 0.01 --no-sleep; then
      record "cpu_oracle_compare:mixed-stacks" "pass" "overlapping statics vs Box3D"
    else
      record "cpu_oracle_compare:mixed-stacks" "fail" "mixed-stacks CPU compare failed"
    fi
  else
    record "cpu_oracle_compare:mixed-stacks" "fail" "oracle dump failed"
  fi
else
  record "cpu_oracle_compare:revolute" "fail" "oracle missing after rebuild"
  record "cpu_oracle_compare:high-resistance" "fail" "oracle missing after rebuild"
  record "cpu_oracle_compare:mixed-stacks" "fail" "oracle missing after rebuild"
fi
rm -rf "$DUMP_DIR"

END_HASH="$(source_fingerprint)"
if [[ "$END_HASH" != "$SOURCE_HASH" ]]; then
  record "fingerprint_stable" "fail" "sources or executable changed during gate"
else
  record "fingerprint_stable" "pass" "$SOURCE_HASH"
fi

status="pass"
if [[ "$fail" -gt 0 ]]; then
  status="fail"
elif [[ "$skip" -gt 0 ]]; then
  status="incomplete"
fi

python3 - "$OUT" "$GIT" "$DIRTY" "$SOURCE_HASH" "$BOX3D_REV" "$pass" "$fail" "$skip" "$CASES" "$status" "$ROOT/correctness-fingerprint-inputs.txt" <<'PY'
import json, sys, hashlib, os
out, git, dirty, source, box3d, pass_n, fail, skip, cases_path, status, inputs_path = sys.argv[1:]
cases = []
with open(cases_path) as f:
    for line in f:
        line = line.strip()
        if line:
            cases.append(json.loads(line))
parts = {"combined": source}
try:
    text = open(inputs_path).read()
    for name, start, end in (
        ("files", "=== files ===", "=== gpu-physics bin ==="),
        ("gpu_physics_bin", "=== gpu-physics bin ===", "=== oracle bin ==="),
        ("oracle_bin", "=== oracle bin ===", "=== rustc ==="),
        ("rustc", "=== rustc ===", "=== adapter ==="),
        ("adapter", "=== adapter ===", None),
    ):
        i = text.find(start)
        j = text.find(end) if end else len(text)
        if i >= 0:
            chunk = text[i:j if j >= 0 else None]
            parts[name] = hashlib.sha256(chunk.encode()).hexdigest()
except OSError:
    pass
doc = {
    "status": status,
    "git": git,
    "dirty": dirty == "true",
    "source_sha256": source,
    "fingerprint_parts": parts,
    "box3d_rev": box3d,
    "fused_islands": False,
    "cpu_win_validated": False,
    "pass": int(pass_n),
    "fail": int(fail),
    "skip": int(skip),
    "cases": cases,
}
with open(out, "w") as f:
    json.dump(doc, f, indent=2)
    f.write("\n")
PY

echo "wrote $OUT (status=$status pass=$pass fail=$fail skip=$skip)"
if [[ "$fail" -gt 0 ]]; then
  exit 1
fi
if [[ "$status" == "incomplete" ]]; then
  echo "gate incomplete: required evidence skipped"
  exit 2
fi
exit 0
