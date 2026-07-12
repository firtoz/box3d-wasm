# Box3D engine patches

Local patches applied on top of the clean `box3d/` submodule when building WASM.

The submodule itself should stay **clean**. `packages/box3d-wasm/scripts/prepare-box3d.ts` exports `git archive` of `box3d` HEAD into `packages/box3d-wasm/.box3d-patched/`, then applies every `*.patch` here in sorted order. Apply failure **fails the WASM build**.

## Current patches

| Patch | Purpose |
|-------|---------|
| `0001-profile-levels.patch` | `b3ProfileLevel` (`off` / `coarse` / `full`) so demo charts can disable engine timers |

`BASE_SHA` records the submodule commit the patches were last refreshed against.

## When bumping the `box3d` submodule

1. Update the submodule pointer as usual.
2. Run `bun run build:wasm` (or `bun packages/box3d-wasm/scripts/prepare-box3d.ts`).
3. If a patch fails to apply: refresh or drop it, update `BASE_SHA`, rebuild.
4. Do **not** leave lasting dirty edits in `box3d/` — fold them into a new/updated patch instead.

## Regenerating a patch

```sh
# Edit files under box3d/, then:
cd box3d
git diff > ../patches/box3d/0001-profile-levels.patch
git rev-parse HEAD > ../patches/box3d/BASE_SHA
git checkout -- .
```

`prepare-box3d.ts` applies patches with `git apply --directory=packages/box3d-wasm/.box3d-patched` from the repo root (plain `git apply` inside the worktree skips hunks against the parent repo).