import { createManifoldHost } from "./manifold-shared";
import { triangleVsSphereScene } from "./triangle-vs-sphere-scene";

export const triangleVsSphereSample = createManifoldHost(
  triangleVsSphereScene,
  () => new Worker(new URL("./triangle-vs-sphere.worker.ts", import.meta.url), { type: "module" }),
);
