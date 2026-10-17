# Actual CPU recording for Replay UI review

The76,546-byte native Box3D recording contains64frames, five stable bodies plus one spawn/delete ordinal, named hulls/spheres/capsule, a revolute joint, contacts and all seven query kinds. Stable repeated query keys and untagged queries are present. [Actual counts, validation evidence, failed probe assumptions and cleanup](replay-fixture-acceptance.json).

The first five query kinds have positive and zero-result frames. Both mover query kinds have empty recorded hit lists in this fixture; Cast Mover deliberately exposes zero hit entries in the native player and retains its separate fraction. Do not interpret zero recorded hits as a qualified no-collision result. UI Kind/Min hits may correctly return no rows.

Native replay validation passed before an overly strict diagnostic coverage assertion failed. The same recording was subsequently scanned read-only; it was not regenerated. The compiler/setup and coverage failures are retained. All helper executables were deleted, child processes reaped, core files absent. No GPU build, viewer, engine or Box3D/WASM modification. This prepares review input and does not close any Replay widget yet.
