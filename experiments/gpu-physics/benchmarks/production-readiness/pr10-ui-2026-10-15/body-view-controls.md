# Body Type and View controls

Actual CPU first, ordinary/native GPU and both combined routes reviewed. [Acceptance and original partial receipts](body-view-controls-acceptance.json), [CPU-first before grid](body-view-controls-grid.jpg), [UI06 transparency](UI06.md), [UI07 framing](UI07.md).

All source-derived type setters reach the intended five bodies per engine; enable targets the upstream three bodies. Actual type/enabled getters and preserved origins/identities are checked. Kinematic platform motion is−0.1m over two1/60s steps at−3m/s independently in each engine, without CPU/GPU scene-trajectory comparison. The first ordinary GPU step took29.896s; the second.176s. Type changes can trigger the visible loading screen, which the corrected harness waits for with finite90s deadlines.

Compound / Hulls retains one physics body and one baked compound shape. Draw Distance100→10→100 changes actual child-instance submissions20→0→20 on every route. Joint/Force scale values change and restore at the actual draw boundary; missing GPU debug geometry stays UI03. Fresh BodyType restores default dynamic/enabled,60Hz,4substeps; identities are invalid during the intervening Hulls world. The original permanent CPU-invalidity assertion across two recreations was a harness assumption and is retained as such.

No newly found issue is fixed. Eight shared View calls reviewed with explicit defects/limitations; BodyType's four sample controls reviewed on all five routes. This does not close full sample interaction reviews or PR10.
