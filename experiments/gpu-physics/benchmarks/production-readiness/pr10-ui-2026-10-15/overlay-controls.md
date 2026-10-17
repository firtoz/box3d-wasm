# Remaining shared overlay outputs

All12 remaining shared world-draw controls reviewed, with real CPU first and ordinary/native standalone/combined GPU. [Acceptance and actual callback inputs/counts](overlay-controls-acceptance.json). All CPU47checks pass; both combined CPU panes produce the expected graphics. Each GPU route has14 missing effects:12controls plus Joint/Force scale output. These extend UI03 and remain unfixed.

CPU Joints produces segments; Joint Extras adds force/torque segments/text; Graph Colors adds colored points. Sleep emits velocity points/text, Body Names emits labels, Islands emits bounds. Actual live contacts produce points, normal/force segments and feature strings. Both anchor selections reach the chosen flag and actual contact points. Joint and Force scale double the actual segment-input digests and restore the original inputs while physics remains paused. GPU emits none of these overlay callbacks, including when its own world has contacts/joints; normal shape callbacks remain8.

No newly found issue fixed, no solver or WASM edit. All shared sourcecall dispositions are reviewed85/85; this includes defects and exclusions, not85passingfeatures. Sample-specific329widgets/23customhandlers and shared picking/dragging/camera inputs remain.
