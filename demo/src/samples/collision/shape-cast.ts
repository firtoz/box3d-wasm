import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import type { DemoSample } from "../types";
import { createShapeCastBodies, shapeCastCamera, shapeCastGroundSize } from "./shape-cast-scene";

const half = shapeCastGroundSize();

export const shapeCastSample: DemoSample = {
  id: "collision/shape-cast",
  name: "Collision / Shape Cast",
  create(runtime, scene, solverParams) {
    const spec: RenderSpec = {
      groundKind: "none",
      groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
      bodies: createShapeCastBodies(runtime),
      camera: shapeCastCamera,
    };

    return createGenericSample(
      "collision/shape-cast",
      "Collision / Shape Cast",
      spec,
      () => new Worker(new URL("./shape-cast.worker.ts", import.meta.url), { type: "module" }),
    ).create(runtime, scene, solverParams);
  },
};
