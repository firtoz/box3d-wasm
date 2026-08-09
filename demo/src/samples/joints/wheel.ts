import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { wheelBodies, wheelCamera, wheelGroundSize } from "./wheel-scene";

const half = wheelGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: wheelBodies,
  camera: wheelCamera,
  info: "wheel joint (suspension spring off, motors/limits off) — cylinder on empty static anchor",
};

export const wheelSample = createGenericSample(
  "joints/wheel",
  "Joints / Wheel",
  spec,
  () => new Worker(new URL("./wheel.worker.ts", import.meta.url), { type: "module" }),
);
