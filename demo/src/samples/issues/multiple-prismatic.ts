import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createMultiplePrismaticBodies, multiplePrismaticCamera, multiplePrismaticGroundSize } from "./multiple-prismatic-scene";

const half = multiplePrismaticGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  // Upstream: empty static anchor only — no ground hull / draw plane.
  groundKind: "none",
  bodies: createMultiplePrismaticBodies(),
  camera: multiplePrismaticCamera,
  info: "6 prismatic-linked boxes (constraintHertz=240, limits ±6) | huge mouse force",
};

export const multiplePrismaticSample = createGenericSample(
  "issues/multiple-prismatic",
  "Issues / Multiple Prismatic",
  spec,
  () => new Worker(new URL("./multiple-prismatic.worker.ts", import.meta.url), { type: "module" }),
);
