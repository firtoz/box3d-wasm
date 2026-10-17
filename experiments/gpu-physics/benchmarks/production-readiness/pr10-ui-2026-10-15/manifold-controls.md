# Manifold controls and mouse-handler review

**9tools /45per-sample widget entries /9custom mouse-handler groups reviewed onall5routes** (real Box3D CPU first). The45entries reuse10inherited sourcewidgetdefinitions acrossManifold/TriangleManifold. Actual mouse inputs change the real cache arguments, collision outputs and draw poses. Each tool coversLMBtranslation,ShiftLMBrotation,MouseUpstopping andAltLMBignoredbytool;135gestures acrossroutes. Scenechanges/restart restoreoriginal no-bodydefaults.

[CPU-first original state grid](manifold-controls-grid.html) · [actualcache/math/render evidence and receipts](manifold-controls-acceptance.json). These are native no-body preview calculations inallviewers, notGPUsolver/collision qualification.

[UI13](UI13.md):4tools offer unused Usecache;7tools offerunused manualfeature radios (32badwidgetbehaviors perroute). [UI14](UI14.md):HullvsHullauto remainsmanualSAT8 onall5routes;TrianglevsHullcorrectlyreturnsautomatic policy. Both issues remainunfixed pendingfullreview.

Two simplex cache effects requiredshallowcontact, notdeepoverlap orseparation: CapsulevsHull actualcache3→0→3, HullvsSphere2→0→2 onall5routes, whilepreviewgeometry staysfixedacrosstoggle. Original20empty-cache predicates were stimulusassumptions, notproductdefects; originalreceiptsunchanged. Source-derived mouseinput model matches frozenactualdeltas at<1e-5 aftermainFOV50overridewasread; final75focusedchecks verifyactualnativeoperations. No solverrepair or tolerancechange.

1344recordedcheckevaluations:165policydefectfindings groupedintoUI13/UI14;20invalidstimuluspredicates resolved bytargetedshallowevidence. Othercheckspositive. CPUfirst7toolswerepreservedafterpicker-reflowassertion; remaining2onlycontinued. Allapps nowterminal;sharedcachegrowth0/core0.
