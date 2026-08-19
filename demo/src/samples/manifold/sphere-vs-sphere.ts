import { createManifoldHost } from "./manifold-shared";
import { sphereVsSphereScene } from "./sphere-vs-sphere-scene";

export const sphereVsSphereSample = createManifoldHost(
  sphereVsSphereScene,
  () => new Worker(new URL("./sphere-vs-sphere.worker.ts", import.meta.url), { type: "module" }),
);
