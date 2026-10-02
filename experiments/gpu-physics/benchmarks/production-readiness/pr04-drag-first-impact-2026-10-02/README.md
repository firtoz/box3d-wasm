# Loaded dragging: current first-impact diagnosis

Both archived current-source paths first exceed the original held-position limit
at frame 227/body 0, during descent toward the floor. This offline review finds
that their bottom corners are clamped near +5 mm while the CPU bottom is
-10.832 mm, with almost identical velocity at that frame. The next frame shows
a large velocity difference. This supports the recorded CCD involvement; it
still does not identify the exact pre-CCD state, fraction or correction.

| Frame | CPU lowest corner | Ordinary GPU | Native GPU | CPU/ordinary position difference | Velocity difference |
| --- | ---: | ---: | ---: | ---: | ---: |
| 226 | +33.613 mm | +32.706 mm | +32.706 mm | 1.056 mm | 0.0000064 m/s |
| 227 | -10.832 mm | +5.000 mm | +5.000 mm | 17.165 mm | 0.0000019 m/s |
| 228 | -8.780 mm | +0.138 mm | +0.138 mm | 10.257 mm | 0.730630 m/s |

The first-impact position difference **fails** the unchanged 5 mm held screen.
The full [original loaded-drag failures](../pr03-drag-order-native-timeout-2026-10-02/README.md)
remain required; this diagnosis does not redefine acceptance or infer instability
from a single trajectory discrepancy. Printed nine-significant-digit poses feed
a float64 rotated-cube corner calculation; these derived figures are not a
higher-precision replacement for original executable assertions.

## Source and prior controls

Box3D `b3FinalizeBodiesTask` classifies CCD by
`max(maxDeltaPosition, maxVelocity * dt) > 0.5 * minExtent`; rotation uses local
vector extents. GPU host CCD classifies by observed COM translation plus a
quaternion angle times scalar maximum extent, capped at the speculative-distance
shell. Those metrics and thresholds differ. Recorded poses are already after
CCD and cannot alone prove which pre-correction classification was taken.
The old no-CCD/scalar cutoff controls demonstrate involvement but are not
production acceptance. The paired old-cutoff control failed its unchanged
settling limit; no blanket rollback is justified.

Actual current CCD/contact/motor/routing sources are unchanged from this trace
producer. The only retained production change since the baseline is the rigid
**distance** branch; `both_pointer_down` creates motor joints, not distance joints.
The new focused distance test is cfg(test) only. Original source/build identity,
commands, setting overrides and independent CPU producer are linked in the
baseline report. Current source excerpts/full files are retained here to audit
applicability; invocation HEAD alone does not identify binaries.

The next discriminating experiment should observe GPU pre/post host CCD and a
CPU read-only finalize/CCD boundary in the earliest faithful impact. Establish
actual classification, TOI fraction/correction and pre-impact contact geometry
before selecting a candidate. Keep the empty-manifold recycling fix, all CCD,
settling/restitution checks and original dragging tolerances. A partial replay
cannot replace the ten-drag full-window gate. Any engine campaign gets a distinct
finite budget first; do not repeat the closed historical controls unchanged.

## Portable evidence

`protocol.json` freezes two archived traces, body-0 window 220–232 and zero builds,
engine processes, candidates or timing. Original compressed traces are retained
byte-for-byte; each has all 3,060 paired body-0 frames and the full five-body/contact
record. `analysis.json` contains the complete 13-frame window for both paths.
`analyze.py` regenerates that host-only analysis; all failed original outcomes
remain preserved. `raw-index.json` hashes every raw file. Box3D/WASM/defaults and
production solver remain unchanged.

```sh
python3 experiments/gpu-physics/benchmarks/production-readiness/pr04-drag-first-impact-2026-10-02/validate.py
```
