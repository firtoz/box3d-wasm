import { createManifoldHost } from "./manifold-shared";
import { triangleVsCapsuleScene } from "./triangle-vs-capsule-scene";

export const triangleVsCapsuleSample = createManifoldHost(
  triangleVsCapsuleScene,
  () => new Worker(new URL("./triangle-vs-capsule.worker.ts", import.meta.url), { type: "module" }),
);
