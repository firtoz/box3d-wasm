# PR02 population fixture — rejected compound identity assumption

The [frozen campaign](protocol.json) stops at its CPU reference, exit -6: the
compound has two public contact IDs with one manifold each. The fixture incorrectly
expected one contact ID with two manifolds. Five builds complete; **one CPU
process and zero of four GPU processes run**. No result is replaced or receives
GPU qualification credit.

[Diagnosis](diagnosis.json) checks both implementations independently: pinned
Box3D `b3CreateContact` includes the compound child ordinal in its pair key, and
GPU `update_registry`/`keyed_contact` deliberately retains that ordinal. Both
contacts share the public parent shape ID. This corrects an identity-contract
assumption; it does not relax a physical tolerance or imply a solver defect.
The rejected source is [preserved](raw/rejected-fixture.cpp), along with actual
compiled header/source input bytes, all five build commands/hashes and the
aborted CPU output in the [receipt](raw/receipt.json) and [raw index](raw-index.json).

This campaign is closed and never resumed. The
[corrected identity campaign](../pr02-population-identities-2026-10-01/README.md)
has a separately frozen contract/budget; the original failure remains visible.
Read the corner dataset's offline validator to check all three retained campaigns.
