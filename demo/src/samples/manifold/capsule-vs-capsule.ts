import { createManifoldHost } from "./manifold-shared";
import { capsuleVsCapsuleScene } from "./capsule-vs-capsule-scene";

export const capsuleVsCapsuleSample = createManifoldHost(
  capsuleVsCapsuleScene,
  () => new Worker(new URL("./capsule-vs-capsule.worker.ts", import.meta.url), { type: "module" }),
);
