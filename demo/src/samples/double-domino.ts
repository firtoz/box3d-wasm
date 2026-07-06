import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { createDoubleDominoBodies, doubleDominoCamera, doubleDominoGroundSize } from "./double-domino-scene";

const half = doubleDominoGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createDoubleDominoBodies(),
  camera: doubleDominoCamera,
};

export const doubleDominoSample = createGenericSample("double-domino", "Stacking / Double Domino", spec, () => new Worker(new URL("./double-domino.worker.ts", import.meta.url), { type: "module" }));
