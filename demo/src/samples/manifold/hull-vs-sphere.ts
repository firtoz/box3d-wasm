import { createManifoldHost } from "./manifold-shared";
import { hullVsSphereScene } from "./hull-vs-sphere-scene";

export const hullVsSphereSample = createManifoldHost(
  hullVsSphereScene,
  () => new Worker(new URL("./hull-vs-sphere.worker.ts", import.meta.url), { type: "module" }),
);
