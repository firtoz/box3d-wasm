# Shared picking, dragging and camera input

RealCPUfirst and fourGPUroutes reviewed with actualmouse/keyboard input on SingleBox. [Acceptance and partialreceipts](picking-camera-acceptance.json), [UI10 capturedkeyup](UI10.md), [UI11 combinedEscape](UI11.md).

Original cube ID remains selected while actualmotor drag moves it. World counts2→3→2 demonstrate transient mouse-body creation/cleanup with only2visible shapes. Panel release and focus loss clean up; held worldgesture retainsmouseownership (WantCaptureMousefalse overInfo). Bothrightpane drag acrossdivider keepsnegative localx andmovesbothindependent bodies in thatdirection. F, staticground/emptyselection, cameraorbit/pan/dolly/flylook/WASDQE, release anddefaultrestoration are checked. No crossengine trajectory/pixel gate.

These shared source paths apply where samples inherit the base controls. The23customhandlers, third-person overrides and329remaining samplewidgets needtheir ownchecks; this evidence doesnotmark165fullreviews complete. No newly found issue fixed.
