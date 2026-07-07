import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { inclinedPlaneBodies, inclinedPlaneCamera, inclinedPlaneGroundSize } from "./inclined-plane-scene";

const half = inclinedPlaneGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: inclinedPlaneBodies,
  camera: inclinedPlaneCamera,
};

export const shapesInclinedPlaneSample = createGenericSample("shapes/inclined-plane", "Shapes / Inclined Plane", spec, () => new Worker(new URL("./inclined-plane.worker.ts", import.meta.url), { type: "module" }));
