# PR02 viewer control stimulus — host proof, apps unlaunched

The single pure X11 receiver process passes. Its observed event sequence is
mouse down/up at `[400,400]`, held **251 ms**, then the expected P key down/up,
held **100 ms**, in the focused receiver window. These are actual received X11
events and timestamps, not a claim based only on synthetic send calls. This
addresses the earlier immediate mouse down/up failure before app controls are
evaluated. No physics engine is initialized by the receiver.

The immutable [protocol](protocol.json) budgets one host receiver and five
serial viewers, real CPU first, for pause/single-step, warm-start/sleep toggles,
restart, actual diagnostic tab contents and clean exit. It is **stopped before
all five viewer apps** because the separate population campaign finds a real
[combined compound-creation defect](../pr02-corner-populations-2026-10-01/README.md).
That adapter needs correction and refrozen viewer inputs. No app gets control,
startup, shutdown or physical qualification credit here.

The [raw index](raw-index.json) preserves the receiver script, server log,
observed-event receipt and frozen protocol; [progress](progress.json) records
one host pass, zero apps, zero candidates and zero timing. Existing old binary
hashes/protocol are never overwritten to masquerade as a fixed build. Later
required-app qualification may reuse directly applicable receiver proof, with
a new finite protocol and exact compiled-input receipts after the fix. The
earlier stopped tab-only campaign is also retained and is not resumed.
