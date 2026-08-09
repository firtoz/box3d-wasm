#!/usr/bin/env bash
# Compare every dump-enabled WASM sample against the C++ reference dumper.
# Soft multi-contact exceptions (documented in docs/SAMPLES.md) are still run,
# but may use a shorter exact-match frame window from scripts/dump-soft-exceptions.json.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_ROOT="${REFERENCE_DUMPS_DIR:-$ROOT_DIR/.reference-dumps/all}"
BUILD_DIR="${REFERENCE_DUMP_BUILD_DIR:-$ROOT_DIR/.reference-dumps/reference-build}"
SOFT_JSON="$ROOT_DIR/scripts/dump-soft-exceptions.json"
FRAMES_DEFAULT="0,50,100,200,300"
EPSILON="${epsilon:-1e-5}"
FAIL_SOFT="${FAIL_SOFT:-0}"

export REFERENCE_DUMP_BUILD_DIR="$BUILD_DIR"
export REFERENCE_DUMPS_DIR="$OUT_ROOT"

mkdir -p "$OUT_ROOT"
SAMPLES_JSON="$(bun "$ROOT_DIR/scripts/wasm-dump.ts" --list-json)"

mapfile -t SAMPLE_IDS < <(SAMPLES_JSON="$SAMPLES_JSON" node - <<'NODE'
const data = JSON.parse(process.env.SAMPLES_JSON);
for (const sample of data.samples) console.log(sample.id);
NODE
)

SOFT_LOOKUP="$(SOFT_JSON="$SOFT_JSON" node - <<'NODE'
const fs = require('fs');
const path = process.env.SOFT_JSON;
const soft = fs.existsSync(path) ? JSON.parse(fs.readFileSync(path, 'utf8')) : {};
process.stdout.write(JSON.stringify(soft));
NODE
)"

PASS=0
SOFT_PASS=0
FAIL=0
SOFT_FAIL=0
FAIL_LIST=()
SOFT_FAIL_LIST=()

TOTAL="${#SAMPLE_IDS[@]}"
i=0
for id in "${SAMPLE_IDS[@]}"; do
  i=$((i + 1))
  frames="$(SOFT_LOOKUP="$SOFT_LOOKUP" ID="$id" FRAMES_DEFAULT="$FRAMES_DEFAULT" node - <<'NODE'
const soft = JSON.parse(process.env.SOFT_LOOKUP);
const id = process.env.ID;
const entry = soft[id];
if (entry && typeof entry.matchThroughFrame === 'number') {
  const max = entry.matchThroughFrame;
  const frames = [];
  for (const f of [0, 50, 100, 200, 300]) {
    if (f <= max) frames.push(f);
  }
  if (!frames.includes(max) && max > 0) frames.push(max);
  frames.sort((a, b) => a - b);
  process.stdout.write(frames.join(','));
} else {
  process.stdout.write(process.env.FRAMES_DEFAULT);
}
NODE
)"
  is_soft="$(SOFT_LOOKUP="$SOFT_LOOKUP" ID="$id" node -e 'const soft=JSON.parse(process.env.SOFT_LOOKUP); process.stdout.write(soft[process.env.ID] ? "1" : "0")')"
  log="$OUT_ROOT/$(echo "$id" | tr '/' '_').log"
  printf '[%d/%d] %s frames=%s%s ... ' "$i" "$TOTAL" "$id" "$frames" "$([ "$is_soft" = 1 ] && echo ' (soft)' || true)"
  if bun run compare:sample -- "sample=$id" "frames=$frames" "epsilon=$EPSILON" >"$log" 2>&1; then
    if [ "$is_soft" = 1 ]; then
      SOFT_PASS=$((SOFT_PASS + 1))
      echo "SOFT_OK"
    else
      PASS=$((PASS + 1))
      echo "PASS"
    fi
  else
    if [ "$is_soft" = 1 ]; then
      SOFT_FAIL=$((SOFT_FAIL + 1))
      SOFT_FAIL_LIST+=("$id")
      echo "SOFT_FAIL"
    else
      FAIL=$((FAIL + 1))
      FAIL_LIST+=("$id")
      echo "FAIL"
    fi
    rg -n 'first divergence|body count|checkpoint count' "$log" | head -3 || true
  fi
done

echo
echo "==== summary ===="
echo "hard pass: $PASS"
echo "soft pass: $SOFT_PASS (early-window OK)"
echo "hard fail: $FAIL"
echo "soft fail: $SOFT_FAIL"
if [ "${#FAIL_LIST[@]}" -gt 0 ]; then
  echo "hard failures: ${FAIL_LIST[*]}"
fi
if [ "${#SOFT_FAIL_LIST[@]}" -gt 0 ]; then
  echo "soft failures: ${SOFT_FAIL_LIST[*]}"
fi

if [ "$FAIL" -gt 0 ]; then
  exit 1
fi
if [ "$FAIL_SOFT" = 1 ] && [ "$SOFT_FAIL" -gt 0 ]; then
  exit 1
fi
exit 0
