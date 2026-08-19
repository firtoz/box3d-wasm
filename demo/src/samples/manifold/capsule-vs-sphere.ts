import { createManifoldHost } from "./manifold-shared";
import { capsuleVsSphereScene } from "./capsule-vs-sphere-scene";

export const capsuleVsSphereSample = createManifoldHost(
  capsuleVsSphereScene,
  () => new Worker(new URL("./capsule-vs-sphere.worker.ts", import.meta.url), { type: "module" }),
);
