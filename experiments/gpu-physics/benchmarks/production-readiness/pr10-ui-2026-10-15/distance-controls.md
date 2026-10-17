# Collision tool review

**11 widget entries and Shape Distance's custom mouse group reviewed on all five routes**, real Box3D CPU first. [Grid](distance-controls-grid.html), [actual GJK/TOI/draw acceptance](distance-controls-acceptance.json), [framing acceptance](empty-world-framing-acceptance.json).

Distance Debug changes/restores actual selected simplex draw geometry while keeping calculation unchanged. Shape Distance exercises both type menus, both radius inputs, indices/world labels, cache, simplex display/index, translation/Shift rotation/Alt-ignore/release and restart defaults. Time of Impact exercises both supported menu choices and unchanged sweeps, then the invalid fourth A option last. These zero-body tools use native C algorithms; GPU solver correctness is outside this batch.

UI15 wrong labels/invalid TOI enum, UI16 hidden active radii, UI17 invalid cached indices and UI18 missing preview framing are confirmed all five routes and remain unfixed. There are9 deliberate negative process faults:5 TOI SIGSEGV and4 GPU-viewer cache SIGILL; the CPU cache input is invalid but survives. None is a clean-quit or feature pass.

388 check evaluations include40 policy failures; original CPU Distance Debug slider-label targeting failure is retained. Only unfinished CPU work ran again. Framing has30 recorded evaluations:5 confirmed failures and5 invalid pan stimulus assumptions (Alt omitted); the original receipts are unchanged. All initial/F/menu capsules remain fully outside independently of the rejected pan assumption. No product change was made.

Current totals:89/342 widgets reviewed,253left;113/165 sample dispositions,52left (19actualfixtures+94shared reuse);10/23 custom input groups,13left;16open issues,2earlierlocalfixes. Shared85/85reviewed includes defects/exclusions. PR10 unchecked.
