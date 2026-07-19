/**
 * Compile-time object-API asserts.
 *
 * Bundlers/Vite can define `__BOX3D_OBJECT_ASSERTS__` to `false` so assert bodies
 * become empty after constant-folding and guards never install (prod squeeze).
 * Defaults to enabled (dev). Demo: `BOX3D_OBJECT_ASSERTS=0` when running Vite.
 *
 * There is no runtime early-out toggle: either guards wrap methods (asserts on)
 * or prototypes stay bare (`setObjectAssertGuardsEnabled(false)` / compile strip).
 */
declare const __BOX3D_OBJECT_ASSERTS__: boolean | undefined;

const COMPILE_TIME_OBJECT_ASSERTS: boolean =
  typeof __BOX3D_OBJECT_ASSERTS__ === "boolean" ? __BOX3D_OBJECT_ASSERTS__ : true;

/** False when the bundler defined `__BOX3D_OBJECT_ASSERTS__` as false. */
export function areObjectAssertsCompiledIn(): boolean {
  return COMPILE_TIME_OBJECT_ASSERTS;
}

/**
 * Whether assert bodies should run when called.
 * Same as compile-in flag (for bundler DCE of `if (!objectAssertsEnabled()) return`).
 */
export function objectAssertsEnabled(): boolean {
  return COMPILE_TIME_OBJECT_ASSERTS;
}

/** @deprecated Use `areObjectAssertsCompiledIn` — runtime early-out was removed. */
export function areObjectAssertsEnabled(): boolean {
  return COMPILE_TIME_OBJECT_ASSERTS;
}
