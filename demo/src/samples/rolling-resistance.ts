import { createGenericSample } from "./generic-host";
import type { RenderSpec } from "./generic-host";
import { createRollingResistanceBodies, rollingResistanceCamera, rollingResistanceGroundSize } from "./rolling-resistance-scene";

const half = rollingResistanceGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createRollingResistanceBodies(),
  camera: rollingResistanceCamera,
};

export const rollingResistanceSample = createGenericSample("rolling-resistance", "Shapes / Rolling Resistance", spec, () => new Worker(new URL("./rolling-resistance.worker.ts", import.meta.url), { type: "module" }));
