#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "$root"
mkdir -p target
# One preparation/build at a time; ordinary source and Cargo.lock stay untouched.
exec 9>target/native-cache-build.lock
flock 9
cargo fetch --locked --manifest-path "$root/Cargo.toml"
python3 scripts/prepare-native-backend.py
python3 - "$root" <<'PY'
from pathlib import Path
import sys
root = Path(sys.argv[1])
base = root / 'target/native-workspace'
work = base / 'experiments/gpu-physics'
work.mkdir(parents=True, exist_ok=True)
for child in root.iterdir():
    if child.name in {'target', 'Cargo.toml', 'Cargo.lock', '.git'}:
        continue
    link = work / child.name
    if not link.exists():
        link.symlink_to(child, target_is_directory=child.is_dir())
(work / 'Cargo.toml').write_text((root / 'Cargo.toml').read_text() + '\n[workspace]\n')
if not (work / 'Cargo.lock').exists():
    (work / 'Cargo.lock').write_bytes((root / 'Cargo.lock').read_bytes())
link = base / 'box3d'
if not link.exists():
    link.symlink_to(root.parents[1] / 'box3d', target_is_directory=True)
PY
if [ "$#" -eq 0 ]; then set -- build --release; fi
command=$1
shift
cargo --config "$root/target/native-backend/cargo-config.toml" "$command" --manifest-path "$root/target/native-workspace/experiments/gpu-physics/Cargo.toml" --target-dir "$root/target/native-cache-build" --features native-command-cache "$@"
