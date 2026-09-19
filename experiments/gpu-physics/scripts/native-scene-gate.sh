#!/usr/bin/env bash
# Native sample validator. Missing/stale/truncated JSON is fail.
# Unsupported is incomplete overall, never a compatibility pass.
# Does not authorize cpu_win_validated.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$ROOT/../.." && pwd)"
MANIFEST="$ROOT/native-scenes.json"
VALIDATE="$ROOT/scripts/native_scene_validate.py"
STAMP="${NATIVE_SCENE_STAMP:-$(date +%Y%m%d-%H%M%S)}"
ART="$ROOT/artifacts/native-scene-${STAMP}"
OUT="${1:-$ROOT/native-scene-latest.json}"
mkdir -p "$(dirname "$ART")"
mkdir "$ART" # Refuse reuse: an old successful JSON must never survive a failed run.

# Keep the same adapter/cache choices as the interactive launcher.
export GPU_SOKOL_SEED="${GPU_SOKOL_SEED:-52977}"
export GPU_SOKOL_VILLAGE_DROP=0

python3 "$VALIDATE" --self-check
python3 "$ROOT/scripts/test_gear_support.py"

echo "building portable CPU/GPU samples with launcher defaults"
python3 "$ROOT/scripts/run-native-samples.py" cpu --build-only --build-info "$ART/cpu-build.json"
python3 "$ROOT/scripts/run-native-samples.py" gpu --build-only --build-info "$ART/gpu-build.json"
# Freeze both binaries, and apply exactly the GPU launcher's runtime defaults.
mkdir "$ART/bin"
python3 - "$ART" <<'PYINFO'
import json, shutil, sys
from pathlib import Path
out = Path(sys.argv[1])
for mode in ('cpu', 'gpu'):
    info = json.loads((out / f'{mode}-build.json').read_text())
    shutil.copy2(info['binary'], out / 'bin' / f'samples_{mode}')
PYINFO
python3 - "$ART/gpu-build.json" > "$ART/gpu-runtime.env0" <<'PYENV'
import json, sys
for name, value in json.load(open(sys.argv[1]))['environment'].items():
    sys.stdout.buffer.write(f'{name}={value}\0'.encode())
PYENV
while IFS= read -r -d '' setting; do export "$setting"; done < "$ART/gpu-runtime.env0"
CPU_BIN="$ART/bin/samples_cpu"
GPU_BIN="$ART/bin/samples_gpu"

{
  echo "stamp=$STAMP"
  echo "git=$(git -C "$REPO" rev-parse HEAD)"
  echo "dirty=$(git -C "$REPO" status --porcelain | wc -l)"
  echo "box3d=$(git -C "$REPO/box3d" rev-parse HEAD)"
  echo "seed=$GPU_SOKOL_SEED"
  cat "$ART/gpu-build.json"
  echo "cpu_bin=$(sha256sum "$CPU_BIN" | awk '{print $1}')"
  echo "gpu_bin=$(sha256sum "$GPU_BIN" | awk '{print $1}')"
  (cd "$ROOT" && find src shaders c_abi scripts native-samples Cargo.toml Cargo.lock \
    -type f ! -name '*.json' ! -name '*.mp4' \
    ! -path '*/build-*/*' ! -path '*/target/*' ! -path '*/.fetchcontent-cache/*' \
    2>/dev/null | sort | xargs sha256sum)
} >"$ART/source-identity.txt"
SOURCE_SHA="$(sha256sum "$ART/source-identity.txt" | awk '{print $1}')"

run_timeout() {
  local sec="$1"
  shift
  if [[ -z "${DISPLAY:-}" ]] && command -v xvfb-run >/dev/null 2>&1; then
    timeout "$sec" xvfb-run -a "$@"
  else
    timeout "$sec" "$@"
  fi
}

run_bin() {
  local bin="$1" id="$2" steps="$3" timeout_s="$4" json="$5" log="$6"
  local warmup=2 drop=0
  if [[ "$id" == "Compound/Village" ]]; then warmup=0; drop=1; fi
  set +e
  (
    export GPU_SOKOL_VILLAGE_DROP="$drop"
    cd "$REPO/box3d"
    run_timeout "$timeout_s" "$bin" \
      --sample-name "$id" \
      --unpaced \
      --health-scan \
      --bench-json "$json" \
      --warmup "$warmup" \
      --timed "$steps"
  ) >"$log" 2>&1
  echo $?
  set -e
}

classify() {
  local scene="$1" timed="$2" gpu_json="$3" cpu_json="$4" gpu_rc="$5" gpu_log="$6" cpu_rc="$7"
  python3 - "$VALIDATE" "$scene" "$timed" "$gpu_json" "$cpu_json" "$gpu_rc" "$gpu_log" "$cpu_rc" <<'PY'
import json, sys
from pathlib import Path
validate, scene, timed, gpu_json, cpu_json, gpu_rc, gpu_log, cpu_rc = sys.argv[1:]
gpu_path = Path(gpu_json)
cpu_path = Path(cpu_json)
def read(path):
    if not path.is_file():
        return None
    try:
        return json.loads(path.read_text())
    except Exception as exc:
        return {"_error": str(exc)}
gpu = read(gpu_path)
cpu = read(cpu_path)
log = Path(gpu_log).read_text(errors="replace") if Path(gpu_log).is_file() else ""
fail = bool(gpu and gpu.get("gpu_fail")) or ("GPU PHYSICS FAILED" in log) or ("refusing compound" in log)
payload = {
  "gpu": gpu,
  "cpu": cpu,
  "scene_id": scene,
  "expected_timed": int(timed),
  "process_rc": int(gpu_rc),
  "cpu_process_rc": int(cpu_rc),
  "log_text": log,
  "gpu_fail": fail,
}
import subprocess
out = subprocess.check_output(["python3", validate], input=json.dumps(payload), text=True)
sys.stdout.write(out)
PY
}

pass=0
fail=0
unsupported=0
incomplete=0
cases=()

while IFS=$'\t' read -r id steps timeout_s; do
  [[ -z "$id" ]] && continue
  slug="$(echo "$id" | tr '/ &' '___')"
  echo "native scene $id ($steps steps)"
  cpu_json="$ART/cpu-${slug}.json"
  gpu_json="$ART/gpu-${slug}.json"
  cpu_log="$ART/cpu-${slug}.log"
  gpu_log="$ART/gpu-${slug}.log"
  cpu_rc="$(run_bin "$CPU_BIN" "$id" "$steps" "$timeout_s" "$cpu_json" "$cpu_log")"
  gpu_rc="$(run_bin "$GPU_BIN" "$id" "$steps" "$timeout_s" "$gpu_json" "$gpu_log")"
  if [[ "$cpu_rc" != "0" ]]; then
    echo "cpu $id failed rc=$cpu_rc (GPU still classified)" >&2
  fi
  result="$(classify "$id" "$steps" "$gpu_json" "$cpu_json" "$gpu_rc" "$gpu_log" "$cpu_rc")"
  status="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["status"])' <<<"$result")"
  detail="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["detail"])' <<<"$result")"
  criterion="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["criterion"])' <<<"$result")"
  echo "  $status: $detail ($criterion)"
  cases+=("$(python3 -c 'import json,sys; print(json.dumps({"name":sys.argv[1],"status":sys.argv[2],"detail":sys.argv[3],"criterion":sys.argv[4],"cpu_rc":int(sys.argv[5]),"gpu_rc":int(sys.argv[6])}))' "native:$id" "$status" "$detail" "$criterion" "$cpu_rc" "$gpu_rc")")
  case "$status" in
    pass) pass=$((pass + 1)) ;;
    ok) pass=$((pass + 1)); status=pass ;;
    unsupported) unsupported=$((unsupported + 1)) ;;
    incomplete) incomplete=$((incomplete + 1)) ;;
    *) fail=$((fail + 1)) ;;
  esac
done < <(python3 - "$MANIFEST" <<'PY'
import json, sys
doc = json.load(open(sys.argv[1]))
for scene in doc["scenes"]:
    print(f'{scene["id"]}\t{scene["steps"]}\t{scene.get("timeout_s", 90)}')
PY
)

# Same-process Village -> Bounce House switch. Village remaining unsupported is not a pass.
switch_json="$ART/switch-village-bounce.json"
switch_log="$ART/switch-village-bounce.log"
set +e
(
  cd "$REPO/box3d"
  run_timeout 120 "$GPU_BIN" \
    --sample-name "Compound/Village" \
    --switch-sample-name "Continuous/Bounce House" \
    --switch-after 8 \
    --unpaced \
    --health-scan \
    --warmup 0 \
    --timed 28 \
    --bench-json "$switch_json"
) >"$switch_log" 2>&1
switch_rc=$?
set -e
switch_status="fail"
switch_detail="rc=$switch_rc"
if grep -q 'memory allocation of' "$switch_log"; then
  switch_detail="allocation abort during same-process Village/Bounce House switch"
  fail=$((fail + 1))
elif [[ $switch_rc -eq 0 ]] && python3 - "$switch_json" <<'PY'
import json, sys
from pathlib import Path
p = Path(sys.argv[1])
if not p.is_file():
    raise SystemExit(1)
doc = json.loads(p.read_text())
print(doc.get("switch_count", 0))
if int(doc.get("measured") or 0) < 20:
    raise SystemExit(2)
if int(doc.get("switch_count") or 0) != 1:
    raise SystemExit(3)
PY
  then
    switch_count="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1])).get("switch_count",0))' "$switch_json")"
    if grep -qi 'refusing compound\|GPU PHYSICS FAILED' "$switch_log"; then
      switch_status="incomplete"
      switch_detail="same-process switch completed (switch_count=$switch_count); Village remains unsupported"
      incomplete=$((incomplete + 1))
    elif python3 "$ROOT/scripts/validate-scene-switch.py" --structure "$switch_json"; then
      switch_status="pass"
      switch_detail="complete Village/Bounce House phase identities, step reset and scene health (no matched CPU trajectory claim)"
      pass=$((pass + 1))
    else
      switch_status="fail"
      switch_detail="invalid/incomplete same-process scene phases or health"
      fail=$((fail + 1))
    fi
  else
    switch_detail="same-process switch failed rc=$switch_rc"
    fail=$((fail + 1))
  fi
cases+=("$(python3 -c 'import json,sys; print(json.dumps({"name":"native:scene-switch","status":sys.argv[1],"detail":sys.argv[2],"criterion":"same-process switch"}))' "$switch_status" "$switch_detail")")

overall="fail"
if [[ "$fail" -eq 0 && "$unsupported" -eq 0 && "$incomplete" -eq 0 ]]; then
  overall="pass"
elif [[ "$fail" -eq 0 ]]; then
  overall="incomplete"
fi

python3 - "$OUT" "$overall" "$pass" "$fail" "$unsupported" "$incomplete" "$SOURCE_SHA" "$ART" "$STAMP" "${cases[@]}" <<'PY'
import json, sys
out, overall, pass_n, fail, unsupported, incomplete, source_sha, art, stamp, *cases = sys.argv[1:]
doc = {
    "status": overall,
    "cpu_win_validated": False,
    "source_sha256": source_sha,
    "artifact_dir": art,
    "stamp": stamp,
    "health_scan": True,
    "pass": int(pass_n),
    "fail": int(fail),
    "unsupported": int(unsupported),
    "incomplete": int(incomplete),
    "cases": [json.loads(c) for c in cases],
    "build": json.load(open(art + "/gpu-build.json")),
    "notes": "Portable launcher configuration recorded in build. Library test counts are not native compatibility proof.",
}
open(out, "w").write(json.dumps(doc, indent=2) + "\n")
print(json.dumps(doc, indent=2))
PY
ln -sfn "$ART" "$ROOT/artifacts/native-scene-latest"
test "$overall" = pass
