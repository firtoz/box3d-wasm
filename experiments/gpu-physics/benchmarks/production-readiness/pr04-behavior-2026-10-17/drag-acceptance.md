# Independent dragging/CCD acceptance, frozen before runs

This is the first current physical dragging audit, not a renewed failed trajectory
experiment. Preserve original `c_abi/both_drag_test.cpp` scene, defaults, independent
CPU/GPU rays, ten sequential drags,1320isolated/3060ground steps and no ground resets.
Use frozen PR06 production libraries, not the new Rain cull candidate.

Source correction to acceptance.md's historical5Hz assumption: actual
`c_abi/both_dual.c:3477` uses a MOTOR joint, linearHertz7.5, dampingRatio1.0,
maxSpringForce=force_scale*mass*|gravity|, with force_scale1000 in this fixture.
The old statement is preserved as an explicitly superseded audit assumption.
Static gravitational spring deflection is g/(2π7.5)^2≈4.50mm, not10mm.

Original physical/function requirements remain unchanged: valid independently
selected body identity, valid mouse joint while held, destroyed mouse joint on
release, finite body states, and each released body's linear/angularspeed<.1
at all ten original release endpoints. Original trajectory-only errors still
print their original FAIL/pass and original values, explicitly tagged deferred;
they do not substitute for an anchor residual.

New independent support screen reuses the original support tolerance: every
vertex of every unitcube on each engine must stay above floorY-.025m at every
ground step. Report first/worst engine/frame/body if it fails. Each engine must
also pass the original finite/no-runaway health conventions: |y|<=200m and speed
<=120m/s. The controlled no-ground fixture starts at100m; no floor screen there.

New physically derived tracking screen: after the45step initial ramp, actual
world-space grabbed local anchor must remain within.20m of that engine's actual
kinematic mouse anchor. Original ramp speed is sqrt((.35/45*60)^2+(2/45*60)^2)
≈2.707m/s. A critically damped7.5Hz servo has ramp lag≈2v/(2πf)=.115m;
one60Hz target-update allowance adds.045m and gravity adds.0045m. A20% margin
rounds to.20m. This is independent physical target tracking, not a relaxed old
CPU/GPU pose delta. The body must follow the original2m lift, not remain on floor.
Record constant-target final30step residual separately without inventing a
second numerical acceptance limit after seeing results.

Report CPU first for every engine comparison, full3060/1320 progress, penetration,
tracking, finite states and original release endpoint assertions. Timing is total
combined CPU+GPU completed-step wall time with observations excluded; report it
as such, not pure GPU time. Required ordinary/native ×order0/1 coverage is explicit.
Per-cell target>=60completed SPS,p95<=33.333ms,max<=250ms under frozen settings.
No parameter/default/geometry changes or production repair before the cause is
shown; a new screen failing CPU too is recorded, not relabeled a GPU-only defect.
