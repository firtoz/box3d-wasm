# Box3D submodule bump checklist

Use this when deciding whether to advance `box3d/` and what to verify afterward so WASM ports keep matching C++ frame dumps and sample coverage stays honest.

Related: [`patches/box3d/README.md`](../patches/box3d/README.md) (patch apply), [`SAMPLES.md`](./SAMPLES.md) (port status), [`reference-dump-plan.md`](./reference-dump-plan.md) (dump tooling).

---

## Do we need a bump?

From the repo root (with network):

```sh
cd box3d
git fetch origin
git rev-parse HEAD
git rev-parse origin/main   # or origin/master if that is upstream's default
git rev-list --count HEAD..origin/main
git log --oneline HEAD..origin/main
```

**Bump when** `HEAD..origin/main` is non-empty and the log includes any of:

- Physics / collision / sensor / joint / solver / math changes under `src/` or `include/`
- New or changed `RegisterSample` entries under `samples/`
- Determinism / FP flags in top-level `CMakeLists.txt` (for example `-ffp-contract`)
- Fixes that affect samples we already ported or plan to port

**Defer when** the only delta is UI/gfx/replay/docs with no `src/` / `include/` / sample-registration impact — still fine to bump for currency, but dump risk is lower.

Record `patches/box3d/BASE_SHA` after any successful bump+patch refresh.

### Snapshot (2026-08-09)

| | |
|--|--|
| Local `box3d` | `781673b` (`Fixes 09`, #104) — also `patches/box3d/BASE_SHA` |
| Upstream `origin/main` | `3fc20f5` (`Follow cam`, #107) |
| Behind | **1 commit** |

That commit is worth taking: mostly samples/gfx + replay follow-cam, but it also tightens sensors (“visitors must be convex”), tweaks `B3_MAX_SHAPE_CAST_POINTS`, adjusts FMA contraction CMake logic, and **adds Bodies / Class Ring**. Existing dump-enabled ports do not use sensors yet, so dump blast radius looks small — still run the full dump sweep after bumping.

---

## After updating: required steps

Do these in order. Keep `box3d/` clean; fold engine edits into `patches/box3d/*.patch`.

### 1. Point the submodule and refresh patches

```sh
cd box3d
git checkout <new-sha>   # usually origin/main
cd ..
git add box3d
bun packages/box3d-wasm/scripts/prepare-box3d.ts   # or bun run build:wasm
```

If a patch fails: refresh or drop it under `patches/box3d/`, update `BASE_SHA`, leave the submodule clean (`git -C box3d checkout -- .`).

### 2. Rebuild WASM and note size

```sh
bun run build:wasm
gzip -c demo/public/wasm/box3d-web.wasm | wc -c
```

If gzipped size moved, update `docs/OTHER_PROJECTS.md` (per-project + 4-way tables) and the “Current size” line in `AGENTS.md`.

### 3. Diff the bump for binding / sample impact

```sh
cd box3d
git diff <old-sha>..<new-sha> --stat
git diff <old-sha>..<new-sha> -- include/ src/ samples/shared/ samples/sample_*.cpp
```

Checklist from the diff:

- [ ] New/changed public C API → bridge (`packages/box3d-wasm/cmake/box3d_web_*.c`) + `packages/box3d-wasm/src/index.ts` + `docs/WASM_API_SURFACE.md` (+ `TYPESCRIPT_API.md` if usage changes)
- [ ] Semantic changes (defaults, filters, sensor rules, constants) → grep ports/docs for assumptions; fix TS scenes if they relied on old behavior
- [ ] New `RegisterSample(...)` → add `[ ]` rows in `docs/SAMPLES.md`, adjust **Easy next ports** / Summary counts when appropriate
- [ ] Removed or renamed samples → update `SAMPLES.md`, demo registry, and any `dumpCppSampleName` / `reference-dump` interaction maps
- [ ] Shared helpers (`shared/*.c`) used by WASM or dumps → rebuild paths still compile

### 4. Keep reference-dump building

```sh
bun run test:reference-dump
```

`tools/reference-dump` compiles upstream sample sources from the submodule. New includes under `samples/` (for example gfx helpers) can break the headless stubs — fix stubs / CMake before trusting compares.

Confirm the new sample appears:

```sh
# after a successful reference-dump build
/tmp/reference-dump-build/reference-dump --list-json | grep -i 'Class Ring'   # example for #107
```

(Adjust the build dir if you use `.reference-dumps/reference-build` via `compare:sample`.)

### 5. Revalidate dump parity (coverage)

There are ~99 dump-enabled scenes (`bun scripts/wasm-dump.ts --list-json`). After an engine bump:

1. Smoke: `bun run compare:sample -- sample="Single Box" clean=1`
2. Prefer a **full sweep** of every dump-enabled id (default frames `0,50,100,200,300`, epsilon `1e-5`). There is no `compare:all` script yet — loop the `--list-json` ids, or batch the previously green set and known soft-drift exceptions from `SAMPLES.md`.
3. On mismatch:
   - Narrow frames (`frames=0,1,2,...,50`) and use `compare:dumps` summary / ULP output
   - Decide: setup/order/float32 bug in our port, intentional upstream behavior change (update TS), or soft multi-contact drift (document window in `SAMPLES.md` only after ruling out setup bugs)
4. Interactive dumps (`dumpInteractionSchedule` / `reference-dump.cpp` hooks): re-run those samples explicitly (Motor Joint, Door, Top Down Friction, Weeble, Bullet vs Stack, Candy Cups, Explosion, …).
5. If sleep hides a later checkpoint, use `--disable-sleep-term` via `compare-sample.sh` where documented (for example far ragdolls).

Do **not** call the bump done until the dump-enabled set is green at the documented tolerances, or every new failure is classified in `SAMPLES.md`.

### 6. Coverage hygiene (samples + docs)

- [ ] `docs/SAMPLES.md` status tables match upstream register list after the bump
- [ ] New easy/dumpable samples considered for **Easy next ports** (prefer `[ ]` + `🔧`)
- [ ] Ported scenes still match upstream geometry/defaults (no silent C++ tweaks)
- [ ] WASM size / API surface docs updated if bindings or binary changed
- [ ] Commit submodule pointer + patches + doc/code fixes together

---

## Notes for the pending `3fc20f5` bump specifically

| Area | Action |
|------|--------|
| Sensors | Visitors must be convex (`src/sensor.c`). No dump-enabled sensor ports yet; when porting Events / Sensor samples, only convex visitors. |
| `B3_MAX_SHAPE_CAST_POINTS` | Now `B3_MAX_HULL_VERTICES`. Relevant once shape-cast APIs are wrapped. |
| FMA CMake | Compiler-ID based `-ffp-contract=off`. Expect little dump delta on Linux clang/gcc (already covered). Rebuild native reference-dump anyway. |
| **Class Ring** | New Bodies sample: capsule ring + heavy gem, `allowFastRotation`, internal 960 Hz stepping. Add to `SAMPLES.md` as `[ ]`; port later (custom step cadence → needs `dumpStep` / worker step hooks). |
| gfx / world text / follow cam | Mostly reference-dump stub / include fallout; no TS demo obligation. |

---

## Done means

- Submodule at intended SHA, patches apply, `box3d/` clean, `BASE_SHA` current
- WASM builds; size docs updated if needed
- `test:reference-dump` green; dump-enabled compares green or documented
- `SAMPLES.md` (and related API/size docs) reflect new upstream reality
