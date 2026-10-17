# Tracking-screen applicability diagnosis

The unconditional new.20m screen remains FAILED on both engines. Original
`drag-physical.cpp` and logs are unchanged. This is a correction to that new
screen's free-servo assumption, not an increase to any original physical limit.

The actual first >.20m event is frame232,t172, while the script is lowering the
mouse after floor impact. CPU/GPU residuals.219247/.221975m both have upward floor
impulses (CPU1333.65N·s plus298.52/897.55, GPU1333.95 plus67.09/745.998).
Worst frame1139,t179: CPUtarget[3.28777361,.344374061,.499997139], actualgrabanchor
[3.68134594,1.00361979,.0319321975], center[3.3435967,.501680493,-.0168786421];
GPUanchor[3.67671084,1.0294286,.0255821627], center[3.35432386,.518315554,-.0312319417].
Actualupwardimpulses CPU2990.45654/GPU3249.54102 plusotherfloorpoints oppose the
motor's attempted motion. Both bodies remain supported and settle onrelease.
The desired anchor is incompatible with the current supported orientation;
finite rotational response/contact load invalidates a free linear-servo model.
CPU reproduction is causal context, not a blanket physical pass.

Final screen retains.20m for all lifting, constant-target and free motion after
step45. Only ground lowering(t>=135) with actual upward contactnormal.y>.5 and
normalImpulse>0 may be classified floor-loaded rather than free. This is checked
at every violating frame, not inferred from one example or a contact count.
Always report unrestrictedpeak/FAIL and number of classifiedloadedframes as well
as freepeak. Constant-target phase has no exemption, so failure to lift remains
caught. Every step still obeys originalfinite/no-runaway/support.025 and every
releaseoriginalspeed/angular<.1 checks; mousebody/jointvalidity is checked while
held. No simulation input/parameter is changed. Keep all original trajectory
failure outputs too.

Qualification uses exact frozen PR06 CAND libraries for the original scenes.
Separate low-speed/mutation CCD checks use the existing compiler-built cull
candidate Rust test executables; their only physics-source difference is the
pure boolean-predicate ordering in broadphase. Disclose that distinction; these
are not frozen PR06 release test binaries or final PR07 coverage. Full29CCD and
related physical regressions remain gateD/PR07, not proved by these two selectors.
