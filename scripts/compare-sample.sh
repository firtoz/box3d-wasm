#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_ROOT="${REFERENCE_DUMPS_DIR:-$ROOT_DIR/.reference-dumps}"
BUILD_DIR="${REFERENCE_DUMP_BUILD_DIR:-$OUT_ROOT/reference-build}"
SAMPLE=""
FRAMES="0,50,100,200,300"
EPSILON="1e-6"
CLEAN=0

usage() {
  printf '%s\n' \
    'Usage: bun run compare:sample -- sample="Sample Name" [frames=0,50,100,200,300] [epsilon=1e-6] [clean=1]' \
    '       bash scripts/compare-sample.sh "Sample Name"' \
    '' \
    'Generated files are written under .reference-dumps/ by default.'
}

for arg in "$@"; do
  case "$arg" in
    sample=*) SAMPLE="${arg#sample=}" ;;
    frames=*) FRAMES="${arg#frames=}" ;;
    epsilon=*) EPSILON="${arg#epsilon=}" ;;
    clean=1) CLEAN=1 ;;
    --help|-h) usage; exit 0 ;;
    *)
      if [ -z "$SAMPLE" ]; then
        SAMPLE="$arg"
      else
        printf 'Unexpected argument: %s\n' "$arg" >&2
        usage >&2
        exit 1
      fi
      ;;
  esac
done

if [ -z "$SAMPLE" ]; then
  usage >&2
  exit 1
fi

SAMPLES_JSON="$(bun "$ROOT_DIR/scripts/wasm-dump.ts" --list-json)"
RESOLVED="$(SAMPLES_JSON="$SAMPLES_JSON" SAMPLE="$SAMPLE" node - <<'NODE'
const data = JSON.parse(process.env.SAMPLES_JSON);
const input = process.env.SAMPLE;
const matches = data.samples.filter((s) => s.id === input || s.id.endsWith(`/${input}`) || s.name === input || s.cppName === input);
if (matches.length > 1) {
  console.error(`Ambiguous WASM dump sample: ${input}`);
  console.error(`Matches: ${matches.map((s) => `${s.name} (${s.id}, C++: ${s.cppName})`).join(', ')}`);
  process.exit(1);
}
const sample = matches[0];
if (!sample) {
  console.error(`Unsupported WASM dump sample: ${input}`);
  console.error(`Supported samples: ${data.samples.map((s) => `${s.name} (${s.id}, C++: ${s.cppName})`).join(', ')}`);
  process.exit(1);
}
process.stdout.write(`${sample.id}\n${sample.name}\n${sample.cppName}\n`);
NODE
)"
mapfile -t RESOLVED_LINES <<< "$RESOLVED"
SAMPLE_ID="${RESOLVED_LINES[0]}"
SAMPLE_NAME="${RESOLVED_LINES[1]}"
CPP_SAMPLE_NAME="${RESOLVED_LINES[2]}"

if [ "$CLEAN" -eq 1 ]; then
  rm -rf "$OUT_ROOT"
fi

SLUG="$(node -e 'const s = process.argv[1].toLowerCase().replace(/[^a-z0-9]+/g, "-").replace(/^-|-$/g, ""); process.stdout.write(s || "sample");' "$SAMPLE_ID")"
SAMPLE_DIR="$OUT_ROOT/$SLUG"
CPP_DUMP="$SAMPLE_DIR/cpp.json"
WASM_DUMP="$SAMPLE_DIR/wasm.json"

mkdir -p "$SAMPLE_DIR" "$BUILD_DIR"

cmake -S "$ROOT_DIR/tools/reference-dump" -B "$BUILD_DIR" -DBOX3D_DOUBLE_PRECISION=OFF
cmake --build "$BUILD_DIR" -j"$(nproc)"

SLEEP_FLAG=""
# Samples with per-frame step callbacks (dumpStep) need to stay awake
case "$SAMPLE_ID" in
  bodies/kinematic|bodies/disable) SLEEP_FLAG="--disable-sleep-term" ;;
esac

"$BUILD_DIR/reference-dump" $SLEEP_FLAG --frames "$FRAMES" "$CPP_SAMPLE_NAME" "$CPP_DUMP"
bun "$ROOT_DIR/scripts/wasm-dump.ts" --frames "$FRAMES" "$SAMPLE_ID" "$WASM_DUMP"
bun "$ROOT_DIR/scripts/compare-dumps.ts" --epsilon "$EPSILON" "$CPP_DUMP" "$WASM_DUMP"

printf 'Compared %s (%s, C++: %s)\n' "$SAMPLE_NAME" "$SAMPLE_ID" "$CPP_SAMPLE_NAME"
printf 'C++ dump:  %s\n' "$CPP_DUMP"
printf 'WASM dump: %s\n' "$WASM_DUMP"
