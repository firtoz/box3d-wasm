import { createManifoldHost } from "./manifold-shared";
import { hullVsHullScene } from "./hull-vs-hull-scene";

export const hullVsHullSample = createManifoldHost(
  hullVsHullScene,
  () => new Worker(new URL("./hull-vs-hull.worker.ts", import.meta.url), { type: "module" }),
);
