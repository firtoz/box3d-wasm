# PR02 direct-X11 controls — CPU complete, event verifier failure retained

The real Box3D CPU app completes25 actual interactions, all nine screenshots and
clean exit. Direct libX11 window enumeration/name lookup and observed input focus
replace the unavailable external helper. Pause, exact single-step, checkbox
world flags, restart and actual diagnostics tabs are recorded numerically and
visually. No production engine source or viewer binary changed.

The original verifier then fails because it expects a moved-body event in a
late post-action sample. The actual event exists on completed-step frame711;
subsequent upstream paused `dt=0` steps clear it. This campaign **stops** at that
verifier failure, before any GPU app. The failed assertion and original verifier
are preserved in the [terminal record](raw/terminal.json) and [original validator](raw/validate-app.py).
The unsuccessful GPU prerequisite invocation launched no app/server and consumed
no GPU trial. This campaign does not retroactively become a successful campaign.

A separately frozen [event-window correction](../pr02-controls-event-window-apps-2026-10-01/README.md)
reuses the exact CPU process/1275 records, requires the actual moved-body event
within the completed single-step action window, and retains every other
assertion. That new offline evaluation passes. The acceptance file in this CPU
folder was added by that later evaluator; its hashes/protocol/source amendment
are recorded there. It is not acceptance by this campaign's original verifier.
No CPU process repeats or replacement trials occur.

[Protocol](raw/protocol-before-runs.json): CPU-first5 apps, at most40 actions and20
screenshots per app,600s watchdog, candidates/timing/retries0. All five exact
viewers come from the [current-source build proof](../pr02-controls-current-builds-2026-10-01/README.md).
Real input uses held mouse/key events and observed XGetInputFocus. Diagnostic
Mesa/Xvfb rendering and read-only observer synchronization cannot establish
performance or desktop render FPS.

The [CPU capture](raw/cpu/clip.mp4), original lossless [observer](raw/cpu/observer.jsonl),
actions, receipts, screenshots and [visual review](raw/cpu/visual-review.json) are
portable. Review confirms actual Profile/Counters/Frame Time contents rather than
inferring selection from screenshot names. These are Single Box core controls;
sample-wide widgets, raycasts, physical/repeat/lifecycle release checks stay open.

Run `python3 validate.py` for offline integrity and retained campaign state.
