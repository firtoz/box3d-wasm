export const oldClear='\t\t\t\t\t\t\tClearSelection();';
export const newClear='#if defined(BOTH_SAMPLES)\n                            both_clear_selection();\n#endif\n'+oldClear;
export const clearHelper=`void both_clear_selection(void) {
    // Selection is independent of an in-progress pointer grab.
    memset(g_drag.selected, 0, sizeof(g_drag.selected));
    extern void SetComparisonSelectedBody(b3BodyId);
    extern void SetSelectedBody(b3BodyId);
    SetComparisonSelectedBody((b3BodyId){0});
    SetSelectedBody((b3BodyId){0});
}
`;
