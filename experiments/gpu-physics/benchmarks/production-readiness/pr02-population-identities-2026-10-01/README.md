# PR02 corrected identities — rejected coplanar mesh stimulus

The [frozen campaign](protocol.json) retains the source-audited compound child
identity rule. Its CPU reference passes sensor/compound assertions, then exits
-6 at the flat grid's multiple-manifold assertion. The observed mesh is one
contact with **one** manifold: coplanar triangle and contact normals cluster
together in pinned Box3D `mesh_contact.c`, as explained in
[diagnosis](diagnosis.json). A flat grid cannot prove manifold-chain coverage.

Five builds complete; **one CPU process and zero of four GPU processes run**.
No GPU pass, replaced failure or extended budget is claimed. The rejected source,
compiled input archive, all build commands/hashes and complete CPU output are
preserved in the [raw index](raw-index.json) and [receipt](raw/receipt.json).
The [original compound-assumption failure](../pr02-populations-2026-10-01/README.md)
also remains unchanged. Neither campaign is resumed.

The next [corner campaign](../pr02-corner-populations-2026-10-01/README.md) uses
the existing three-plane relocation fixture to exercise its original >=3 patch
criterion, rather than weakening that criterion. Its offline validator checks
the retained files and stop rules for all three datasets.
