# PR10 — native sample UI and widgets

**Complete and verified locally,2026-10-16.** All165native samples have interaction dispositions;342active samplewidgets,85shared calls and23custom inputgroups are reviewed.29repairgroups are verified,0confirmedPR10repairgroups remain. [Acceptance matrix](final-acceptance.json) and[recovery log](log.md).

Coverage includes71actualspecific-controlfixtures and94explicitshared-code reuses. It is not165newruntimepasses. [Per-sample applicability](sample-applicability.md),[inventory](inventory.json) and[issue registry](issues.json) retain original findings, exclusions and limits. Historical stage counts in individual reports are superseded by this final audit.

| Repair | Result | Evidence |
| --- | --- | --- |
| [UI01](UI01.md) | readable Info panel text | [Verified evidence](ui01-acceptance.json) |
| [UI02](UI02.md) | panel wheel also moved the camera | [Verified evidence](ui02-acceptance.json) |
| [UI03](UI03-repair.md) | GPU debug overlays | [Verified evidence](ui03-acceptance.json) |
| [UI04](UI04-repair.md) | CPU Profile label width | [Verified evidence](ui04-acceptance.json) |
| [UI05](UI05-repair.md) | hidden HUD layout and combined control visibility | [Verified evidence](ui05-acceptance.json) |
| [UI06](UI06-repair.md) | current body type and CPU draw ownership | [Verified evidence](ui06-acceptance.json) |
| [UI07](UI07-repair.md) | Frame Camera uses the actual split viewport | [Verified evidence](ui07-acceptance.json) |
| [UI08](UI08-repair.md) | Replay filename on Unix | [Verified evidence](ui08-acceptance.json) |
| [UI09](UI09-repair.md) | Honest Replay availability | [Verified evidence](ui09-acceptance.json) |
| [UI10](UI10-repair.md) | captured releases clear camera and sample-host key state | [Verified evidence](ui10-acceptance.json) |
| [UI11](UI11-repair.md) | Escape clears the combined selection owner | [Verified evidence](ui11-acceptance.json) |
| [UI12](UI12-repair.md) | Box Hull null preview guarded | [Verified evidence](ui12-acceptance.json) |
| [UI13](UI13-repair.md) | Show only supported Manifold controls | [Verified evidence](ui13-acceptance.json) |
| [UI14](UI14-repair.md) | Auto SAT policy clears the previous manual feature | [Verified evidence](ui14-acceptance.json) |
| [UI15](UI15-repair.md) | TOI menus match their supported shapes | [Verified evidence](ui15-acceptance.json) |
| [UI16](UI16-repair.md) | Hidden radii stop affecting unrounded previews | [Verified evidence](ui16-acceptance.json) |
| [UI17](UI17-repair.md) | Invalidate cached support indices when a proxy changes | [Verified evidence](ui17-acceptance.json) |
| [UI18](UI18-repair.md) | Frame actual no-body preview geometry | [Verified evidence](ui18-acceptance.json) |
| [UI19](UI19-repair.md) | Independent Wheel steering damping | [Verified evidence](ui19-acceptance.json) |
| [UI20](UI20-repair.md) | Driving rear motor torque | [Verified evidence](ui20-acceptance.json) |
| [UI21](UI21-repair.md) | Release ragdoll anchors on respawn | [Verified evidence](ui21-acceptance.json) |
| [UI22](UI22-repair.md) | Keep Door limits consistent with the checkbox | [Verified evidence](ui22-acceptance.json) |
| [UI23](UI23-repair.md) | Update GPU mesh collision geometry and its cached drawing | [Verified evidence](ui23-acceptance.json) |
| [UI24](UI24-repair.md) | Preserve flat Height Field floors | [Verified evidence](ui24-acceptance.json) |
| [UI25](UI25-repair.md) | Keep complete text input through GPU preparation | [Verified evidence](ui25-acceptance.json) |
| [UI26](UI26-repair.md) | Preserve Tree Load recovery | [Verified evidence](ui26-acceptance.json) |
| [UI27](UI27-repair.md) | Position Mesh Scale sphere previews from their cast start | [Verified evidence](ui27-acceptance.json) |
| [UI28](UI28-repair.md) | Expose the active Box cast size | [Verified evidence](ui28-acceptance.json) |
| [UI29](UI29-repair.md) | Allow Auto Generate to stop during GPU regeneration | [Verified evidence](ui29-acceptance.json) |

Each linked report retains its CPU-first before/after screenshots and actual input/effect checks. [Media identity manifest](final-media-manifest.json). GPUdebugdiagnostics andnativeReplay exclusions remain explicit; original harness failures are not relabelled successful runs. [Observation dispositions](final-observations.json) keep BodyCast queryappearance unconfirmed underPR11.

[Five fresh native builds/startup checks](final-compatibility.json),[full typechecking/demo build](final-types-dependency.json) and[isolated unchanged-source WASM runtime check](final-wasm-compatibility.json) passed. The root now declares the existing Buntypes catalog required by its scripts. SDK/build outputs and consumed diagnostic binaries were removed; sources, hashes and evidence remain. [Cleanup](final-cleanup.json).

**Commit and push authorized.** The user removed mandatory recording/comparison prerequisites on2026-10-17. [Guidance](../../../../../AGENTS.md) now treats captures as optional diagnostics. Unnecessary publication clips/sheets and their orchestration were deleted; the verified repair reports and before/after screenshots are preserved. [Five additional fresh native builds](publication-checks.json), [cleanup](publication-cleanup.json), [portable selected-media mapping](portable-media.json). Historical JSON media paths resolve through that mapping; raw run trees remain excluded. PR11, original Rain physical failures and other release gates remain open.
