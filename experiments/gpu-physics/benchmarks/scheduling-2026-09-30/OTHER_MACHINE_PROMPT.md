# Bounded scheduling validation on the second machine

Validate this scheduling choice on the Ryzen 9 8945HS / RTX 4070 Laptop, without
claiming that the desktop results apply there. Work on `feat/gpu`, preserve local
work, fetch/integrate remote changes, and read `docs/gpu-scheduling-goal.md` first.
Do not resume Rain or the 100–200k scaling sweep. Commit/push the resulting
portable evidence only if the user authorizes doing so on that machine.

Use exactly 50,000 falling cubes and 4,096 mixed-stack bodies, native Vulkan on
the actual NVIDIA adapter, four substeps, sleep disabled, color prefix 20,
90 warmup and 240 timed steps, three fresh-process trials with alternating order.
Do not run builds, tests, other GPU work or recordings alongside timing. Preserve
all trials and failures; do not selectively extend an unfavorable result.

From `experiments/gpu-physics` (choose a fresh output directory):

```sh
bash scripts/build-native-cache.sh build --release --bin gpu-physics > artifacts-scheduling-build.log 2>&1
cmake -S oracle -B oracle/build -DCMAKE_BUILD_TYPE=Release
cmake --build oracle/build -j8
mkdir -p artifacts/scheduling-laptop/binaries
cp target/native-cache-build/release/gpu-physics artifacts/scheduling-laptop/binaries/current
python3 scripts/bench-bounded-scheduling.py artifacts/scheduling-laptop/baseline \
  --binary artifacts/scheduling-laptop/binaries/current --cpu
python3 scripts/bench-bounded-scheduling.py artifacts/scheduling-laptop/confirmation \
  --binary artifacts/scheduling-laptop/binaries/current \
  --candidate-binary artifacts/scheduling-laptop/binaries/current
```

Both controls and automatic run the same frozen current build on this machine;
controls explicitly force their schedule. This checks the policy choice, rather
than a compiler/build speedup. Record its source revision, engine source hashes,
clean/dirty source patch, exact build command/log, executable SHA256 and native
backend configuration in `baseline-build.json` and `candidate-build.json`.
Both receipts should carry the same binary hash. Runtime checkout metadata alone
is insufficient. This copyable helper creates the source receipt (keep the build
log separately):

```sh
python3 - <<'PY'
import hashlib, json, subprocess
from pathlib import Path
root=Path.cwd(); out=root/'artifacts/scheduling-laptop'
files=subprocess.check_output(['git','ls-files','src','shaders','compiler',
    'Cargo.toml','Cargo.lock','build.rs','scripts/build-native-cache.sh',
    'scripts/prepare-native-backend.py'],text=True).splitlines()
patch=subprocess.check_output(['git','diff','--','src','shaders','compiler','Cargo.toml','Cargo.lock'])
(out/'source.patch').write_bytes(patch)
receipt={'revision':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
 'binary_sha256':hashlib.sha256((out/'binaries/current').read_bytes()).hexdigest(),
 'engine_sources':{f:hashlib.sha256((root/f).read_bytes()).hexdigest() for f in files if (root/f).is_file()},
 'build_command':'bash scripts/build-native-cache.sh build --release --bin gpu-physics',
 'patch_sha256':hashlib.sha256(patch).hexdigest()}
for name in ['baseline','candidate']:
 (out/(name+'-build.json')).write_text(json.dumps(receipt,indent=2)+'\n')
PY
python3 scripts/publish-bounded-scheduling.py artifacts/scheduling-laptop \
  benchmarks/scheduling-laptop
python3 scripts/plot-bounded-scheduling.py benchmarks/scheduling-laptop --validate-only
python3 scripts/plot-bounded-scheduling.py benchmarks/scheduling-laptop
```

The publisher permits absent desktop-only diagnostic data; retain any laptop
diagnostics separately and do not pool them with headline timing. The compatible
format is `results.json` plus the SHA256-checked `raw.json.gz` text-file bundle.
The plots use the actual adapter and CPU from those raw records, with orange CPU
and distinct GPU colors. Keep this dataset separate from both the desktop report
and existing machine scaling results.

Run the focused policy tests and existing equivalence/replay/island/capacity
selections separately; use the desktop raw `run-checks.py` commands with native
configuration and explicit component=1 for the legacy control fixtures. Do not
relax their tolerances. Report median trial mean completed-step time, steps/s,
p 95, all trial ranges and adapter/driver. Compare auto against BOTH explicit
component and global: require at least 10% lower median time on one fixture and
no greater than 5% median or p 95 regression on the other. A failed criterion is
an unqualified second-machine result, not a reason to change thresholds or erase
trials. Do not claim whole-engine qualification.
