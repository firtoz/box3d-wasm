# PR02 completed-step event window — CPU accepted, real GPU lifetime failure

One offline correction evaluates the existing CPU1275-record/25-action process,
without another CPU launch. It passes the actual moved-body requirement using
its completed single-step window, plus every unchanged flag, pause, exact-step,
restart, tab-review and exit assertion. The [new frozen protocol](raw/protocol-before-runs.json)
records both verifier hashes and the upstream reason: `Sample` sets
`m_stepWhilePaused=true`; later `b3World_Step(dt=0)` clears transient body events.
The [offline receipt](raw/offline-cpu-receipt.json) hashes the original CPU evidence
and corrected acceptance. No tolerance or required behavior is relaxed; the
original failed assertion remains in the [closed earlier campaign](../pr02-controls-focus-apps-2026-10-01/README.md).

The first ordinary GPU app then demonstrates a **real world-lifetime defect**.
Pause and single-step pass; all three off checkboxes reach actual GPU world flags.
Clicking Restart resets the world/sample/completed steps from146 to0 while all
flags remain off, but the world ID stays **[1,1]**. The required distinct lifetime
check fails. The app exits cleanly after deliberate Ctrl+Q at that stop;12 actions,
no GPU acceptance. Native GPU and both combined app cells remain **unlaunched**.
The [terminal receipt](raw/terminal.json) retains exact before/after states.

Independent [root-world C diagnosis](../pr02-world-lifetime-diagnostic-2026-10-01/README.md)
then confirms stale handles revive and can read/change/destroy replacement worlds
on all four GPU configurations. The source always reuses generation1 and destroys
without checking generation. This is not a tab/stimulus error. PR01 lifecycle
acceptance is reopened; PR02 and PR05 remain incomplete.

Budget: offline correction1/CPU evaluation1/freshCPU0/first GPU apps4/timing0/
candidates0/retries0, stop at first failure. Only one GPU process consumed. Exact
five [current-source viewers](../pr02-controls-current-builds-2026-10-01/README.md),
physics defaults/settings and held input are unchanged. Bounded wheel scrolling
reaches the GPU Info panel controls below the diagnostic sidebar. GPU-specific
automatic-scheduling and recording-unavailable labels are visible, although long
sidebar/recording text is clipped at this1280x720 layout; broader PR10 layout checks
remain open. GPU tab/on-toggle checks are not run after the lifetime stop.

The [full short capture](raw/ordinary-gpu/clip.mp4), [observer](raw/ordinary-gpu/observer.jsonl),
actions, PNGs and logs are portable. Vulkan physics uses actual NVIDIA; drawing
uses Mesa software/CPU pose mirrors. Diagnostic profiles/cadence are not timing.
Run `python3 validate.py` for offline integrity, unchanged verifier assertions,
corrected CPU acceptance and exact retained GPU lifetime failure.
