# Disable controls review

Both Enable Link and Enable Ball pass on real CPU, ordinary/native GPU and both combined routes: **155 actual checks**. Setter snapshots show the same valid body IDs and original pose, changed enabled flag, and preserved paused world. Disabled bodies remain fixed through2actualsteps; the enabled link participates while ball remains fixed; reenabled ball falls on each own engine. Restart restores both enabled controls and6bodies/6shapes at60Hz/4substeps.

[CPU-first state grid](disable-controls-grid.html) · [API calls, poses and acceptance](disable-controls-acceptance.json) · [TypeScript driver](disable-controls.ts). No product fixes or new issues. These focused steps qualify the controls; they do not assert long-window solver correctness or exact CPU/GPU trajectories.
