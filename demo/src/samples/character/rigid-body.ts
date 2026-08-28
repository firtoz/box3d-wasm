import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createRigidBodyBodies, rigidBodyCamera, rigidBodyGroundSize } from "./rigid-body-scene";

const half = rigidBodyGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: createRigidBodyBodies(),
  camera: rigidBodyCamera,
  info: "WASD locked rigid-body character",
  drivePad: true,
};

export const rigidBodySample = createGenericSample(
  "character/rigid-body",
  "Character / Rigid Body",
  spec,
  () => new Worker(new URL("./rigid-body.worker.ts", import.meta.url), { type: "module" }),
);
