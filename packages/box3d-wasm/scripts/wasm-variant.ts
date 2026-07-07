export type WasmBuildVariant = "release" | "profile" | "growable";
export type WasmBuildMode = WasmBuildVariant | "all";

const VALID_VARIANTS: readonly WasmBuildVariant[] = ["release", "profile", "growable"];

export function parseWasmBuildMode(raw: string | undefined): WasmBuildMode {
  const value = raw?.trim().toLowerCase();
  if (value === "profile" || value === "growable" || value === "all") return value;
  return "release";
}

export function getWasmBuildMode(): WasmBuildMode {
  return parseWasmBuildMode(process.env.BOX3D_WASM_VARIANT);
}

export function variantsToBuild(mode: WasmBuildMode): WasmBuildVariant[] {
  if (mode === "all") return [...VALID_VARIANTS];
  return [mode];
}

export function isWasmBuildVariant(value: string): value is WasmBuildVariant {
  return VALID_VARIANTS.includes(value as WasmBuildVariant);
}
