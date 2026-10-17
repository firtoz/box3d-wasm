# PR05 and PR06 — current recovery

Read this FIRST on continuation/compaction/restart, then AGENTS.md and docs/goals/gpu-production-readiness.md. Use goal-scratchpad.

## Objective and authorization

User activated PR05 runtime safety followed by PR06 reproducible release builds/consumer integration on2026-10-17, after moving PR04 behind PR06. Complete both milestones, then stop. No new commit/push request. HEAD7792af8f7 onfeat/gpu; preserve the existing local roadmap reorder and all user work. Keep Box3D/WASM/scene geometry/defaults unchanged. No PR04 solver repair, old exhausted experiment, full sample sweep or recording campaign. New automationTypeScript/Bun.

## Acceptance — PR05 5/8, PR06 2/5

| Gate | Required evidence | State |
| --- | --- | --- |
| P1 Bounds/layout/reservation | Relevant C/Rust/shader layouts, overflow/dispatch limits and growth checks | PASS |
| P2 Identity/ownership/replacement | Epoch retirement, invalid/stale handles, body/shape/joint replacement and multiworld isolation | PASS |
| P3 Callback and event ordering | Supported reentry/read contracts, rejected mutations, unlocked query visitors, ordering/lifetime | PASS |
| P4 Async/replay/failure | Pending/completed status, mutation invalidation, sticky failure/clear semantics | PASS |
| P5 Repeated lifecycle | Create/destroy/scene swaps, sleep/wake,pause/single-step,start/shutdown during loading | OPEN |
| P6 Launch/cache matrix | Ordinary/native launch paths with caching on/off | OPEN |
| P7 Resources/concurrency | Thread/FD/process/device-resource ownership review and bounded repeated checks | PASS |
| P8 Reconciliation | Evidence applicability, truthful support limits, docs and cleanup | OPEN |
| B1 Clean CPU target | Fresh native CPU build, adapter/startup receipt | PASS |
| B2 Clean ordinary targets | Fresh Rust/native GPU and combined builds with pinned dependencies | PASS |
| B3 Clean native targets | Fresh native-cache Rust/GPU/combined builds and actual cache/replay controls | OPEN |
| B4 Consumer/compatibility | External native world create/step/read/destroy; TypeScript/demo/WASM compatibility | OPEN |
| B5 Freeze/recovery | Exact compiled inputs/toolchains/binaries, reproducible commands, cleanup/release applicability | OPEN |

## Current findings

PR02 supplies fresh named API contracts and current library identities, not full PR05. Existing epoch/allocation/replay/state tests should be reused. Source locking review: ordinary world getters acquire WORLDS unconditionally, while step invokes contact callbacks under that lock. Shape flag/userdata reads use owned snapshots; diagnostics use try_lock. Potential unsupported getter/mutator reentry can deadlock and C metadata cleanup can occur before a rejected Rust mutation. Need a small frozen baseline before any repair. Confirmed by callback-baseline-receipt.json: actualCPU completes, bothGPU processes hit their8s timeout exactly after diagnosticBUSY2 and shape-valid1, before owner read returns. Original source/library hashes and logs retained.

## Frozen initial probe budget

Two standalone C fixture links<=25s each and two ordinary/native GPU processes<=8s each,zero engine/viewer builds,candidates/retries/captures. One callback reads public shape owner under contact step; print entry/diagnostic BUSY before the smallest suspected re-lock. Expected bounded timeout is evidence,not a pass. Up to16MiB selected logs,one<=64MiB temporary executable at a time,delete immediately after consumer,1GiB total temporary/64MiB retained ceiling,20GiB free floor,core0,owned group termination/reaping. Source/library/protocol hashes before launch. Shared caches measured before/after. Subsequent repair/check/clean-build budgets must be frozen separately before execution.

## Process and storage state

No live owned job. Initial probe CLOSED; all processes reaped, temporary probe deleted. Shared Rusttargets1.7GiB ordinary/1.2GiB native; native backend source9.7MiB. Free home space1.6TiB. Do not delete shared caches or user work. Raw task output locally ignored before writing.

## Next action

Run minimal callback-owner baseline, then retain/fix only demonstrated runtime defects. Audit reusable exact test selectors and PR05 coverage. Freeze any implementation/test builds before launch. PR06 follows only after PR05 closes, with separate bounded clean builds and cleanup.

Initial probe protocol frozen before launch: callback-baseline-protocol.json also includes one actual Box3D CPU control (one25slink/one5sprocess). GPU baselines remain two8stimeouts, no builds/retries/captures. One temporary executable consumed/deleted serially; cache sizes captured.

## Frozen runtime repair budget

One candidate for demonstrated registry reentry cause only: owned callback owner/body/world identity snapshots; same-thread recursive registry access returns explicitbusy instead of hanging; C wrappers reject unsupported callback access before metadata or CPU-mirror mutation. Preserve ordinary noncallback semantics. Shared API-error storage bridges C and Rust; no shader/solver/default change. Up to2Rust library builds+2Rust test builds (180s each/jobs2),4incremental adapters90s each,one test-list process/backend5s. Up to64selected existing Rust test processes/backend90s each within one900s check batch; exact selected list frozen after listing and before tests. Up to4repaired callback processes30s each plus targeted lifetime/query/error/resource fixtures (exact finite cases frozen before launch). Stop on failure,retain original result,do not retry completed cells. Default1GiB temporary/64MiB retained cap, one<=64MiB fixture executable at a time; shared build growth measured and included. Core0,all subprocess groups bounded/reaped; existing shared caches preserved. PR06 clean build budget remains unfrozen pending PR05.

Second confirmed runtime defect: device-loss-baseline-receipt.json uses actual isolated device.destroy(), then observes invalid=false/reported=false after completion; process exits2 as expected failure. Source handler logs errors but does not publish terminal failure. Freeze one device-failure propagation candidate (shared device fault flag, no shader/math/default edits),2library+2test builds180s/jobs2 using current shared caches,then same minimal probe on both backends10s each and selected failure/reentry regressions. Distinct runtime-device-build phase900s total,temporarygrowth included in1GiB cap. Existing callback candidate builds PASS; prior hashes now historical and not reread as current. No GPU runtime tests from the callback candidate are repeated because none were launched beyond the baseline probes.

Runtime final candidate build PASS: bothlibraries/tests and4adapters; exact test binaries in test-executables.json. runtime-rust-protocol.json freezes41selectors/backend (82processes,total900s,90seach; hostgroups have exact expected counts from fresh list). Covers layout/overflow,epochs,lifetimes,reentry/device loss,ownership/events,growth,async/status,replay invalidation and preparation. No old long replay/Rain/scheduling run. Checks require nonzero exact selected counts,not merely exit0.

Runtime C matrix frozen before launch: runtime-c-protocol.json,34fresh linked fixtures/fourroutes (8each+2mapped cleanup),600s total,25slink/60sGPU/10shost,zero Rust/adapter builds. Includes12lifecyclecycles perroute with FD/thread counts and exactzero owned worlds/geometry. One executable at a time/deleted after eachconsumer. No source/candidate change during tests.

Main runtime Rust/C matrices PASS:82Rust selector processes/98selected assertions suites (49tests/backend),34C processes,allunchangedlimits. Fourroutes each12cycles: FD24/24 and threads6/6 baseline/max; zeroownedworlds/geometry. Callback rejected mutations preserve GPUIDs and actual mappedCPU gravity/velocity. Actual device-destroy regression passes onbothbackends,shared worlds stay terminal and independentdevicehealthy. Source/manual review keeps C adapter scene metadata under exclusive scene-owner thread; independent Rust world access serialized; callbacks may not join a thread waiting for same registry.

Supplemental runtime-cache-protocol.json frozen:6previously unlaunched actual dispatch/replay assertions (native48step trace includes replay-hit/invalidation/reentry checks),plus4cache-disabled lifecycle configurations;450s total,90stest/60sC/25slink,no builds,candidates or repeats. Cache-on settings scoped separately; cache-off omits pipeline-cache directory and sets command-cache controls0 while preserving solver settings. Tiny trace temporaryfiles use taskraw and are consumed/removed bytest.

Supplemental cache phase CLOSED on real diagnostic failure: actual replay key producer appends2u32+8u8=16bytes; full-state serializer still expected14 after two earlier specialization flags were added. runtime-cache-receipt.json/log preserve first capturefailure; raw empty. Correct typed shared flag count/decoder length,keeping replay bytes and all captured semantic fields. One native test rebuild<=180s (replay-diagnostic-build-protocol.json),then corrected48step replay capture plus onlypreviously unlaunched2nativecache selectors/4cache-offCcells. No main82Rust/34Cpasses repeated. Production key bytes unchanged; prior runtime evidence needs explicit diagnostic-only applicability review; PR06cleanbuild compiles finalsources.

Runtime supplemental CLOSED/PASS:48-step replay capture16-byte trailer/currentbindgroup and original replay assertions; two native cache mutation regressions and all4cache-off12-cycle C routes. Raw trace consumed. Frozen release-rust-protocol.json:2empty Rust targets,600s each/jobs2,1300s phase;5empty CMake targets will follow under separate protocol before launch. Global clean-build exception6GiBtemporary,512MiBignored candidate binary/library retention for PR07+consumer,64MiBtracked evidence;free20GiB;all consumed build dependency/object trees removed. No old compiled dependency reused; downloaded pinned source caches allowed. No new scene/solver/default changes. PR05 remains5/8 pending launcher/loading checks;PR06 0/5.

Clean release-rust CLOSED/PASS:both empty targets built locked/offline/external-c-shim,replay-diagnostics(+native-command-cache); libraries/actual2397+dep-info sources frozen. release-native protocol frozenbeforelaunch:5independent empty CMake dirs,CPUfirst,45sconfigure/240sbuild/jobs2/30sreceipt each,1650sphase;receipt captures compiled source/header hashes/generatedsource copies/commands/libraries/binaries then deletes each objecttree. Current driver toolsession28378;read release-native-receipt.json andverifyPIDbefore resume. Rust targets stilltaskraw pending generatedmetadata/cleanup.
Additional runtime protocols frozenbyfreeze-runtime-protocols.ts:5externalconsumer CPUfirst/4fresh callback guards;9viewers120seach with10singlesteps+oneBodies/Disable switch;4WM_DELETE loading-worker shutdowns90s;no retries/captures/sweep. WASMoriginalgeneratedpathsandSDKabsent;release-wasm-protocol.json freezesoneisolatedSDK6.0.2downloadinstall/unchangedsourcebuildandactualruntime+demo build(750s),3GiBSDK within6GiBtaskcap,officialgeneratedpathsrestoredabsent;Rustobjectsconsumedfirst. CompatibilityTS/demo190s. No newengineeditsduringcleanbuilds.

Fresh CPU release build+viewer PASS:18measuredframes/10singlesteps/zeroidleadvances/1scene switch/cleanshutdown. release-cpu-viewer is the CPU-first partition of the frozen9cells, not an addedrun. PR05 5/8,PR06 2/5. Viewerfixtures use owned scratch workingdirectories to preserve all user settings. No compiler overrides detected in inherited build environment.

Ordinary clean GPU+combined routes and4cacheon/off viewer cells PASS:18frames/10singlesteps/0idleadvances/1switch each. These are partitions of original9cells, no repeats. PR06 2/5. Remote boundedfetchsuccess,HEAD...origin2ahead/0behind;nointegrationneeded. Native GPU buildcontinues session28378.
