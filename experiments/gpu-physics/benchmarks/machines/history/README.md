# Historical 50k controls

These datasets use the GPU executable built in a detached `49b024f` worktree,
with its pinned Box3D submodule, and the same benchmark runner/settings as current
code. The component run is one diagnostic trial; the global confirmation uses
three fresh processes. No historical laptop timing was reused.

The old executable reports the invocation checkout as its `git` field. That is
not the compiled revision. Raw outputs are retained unchanged. Independent
checkout/build revision and executable SHA256 records are in
`../../cube-regression/build-revisions.json`; `plot-cube-regression.py` checks
those binary hashes before labelling the chart. The raw source hash also does
not attest compiled source through the native-workspace symlink layout.
