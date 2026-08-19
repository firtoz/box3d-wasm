import { createManifoldHost } from "./manifold-shared";
import { capsuleVsHullScene } from "./capsule-vs-hull-scene";

export const capsuleVsHullSample = createManifoldHost(
  capsuleVsHullScene,
  () => new Worker(new URL("./capsule-vs-hull.worker.ts", import.meta.url), { type: "module" }),
);
