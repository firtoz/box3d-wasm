import { createManifoldHost } from "./manifold-shared";
import { triangleVsHullScene } from "./triangle-vs-hull-scene";

export const triangleVsHullSample = createManifoldHost(
  triangleVsHullScene,
  () => new Worker(new URL("./triangle-vs-hull.worker.ts", import.meta.url), { type: "module" }),
);
