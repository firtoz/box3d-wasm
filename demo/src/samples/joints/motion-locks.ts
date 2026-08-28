import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { motionLocksBodies, motionLocksCamera, motionLocksGroundSize } from "./motion-locks-scene";

const half = motionLocksGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: motionLocksBodies,
  camera: motionLocksCamera,
  info: "motion locks + L impulse on the distance-joint body",
  hotkeys: [{ keys: ["l", "L"], message: { type: "impulse" } }],
  controls: [
    { type: "toggle", label: "Lock Linear X", message: { type: "lock-linear-x" }, value: false },
    { type: "toggle", label: "Lock Linear Y", message: { type: "lock-linear-y" }, value: false },
    { type: "toggle", label: "Lock Linear Z", message: { type: "lock-linear-z" }, value: false },
    { type: "toggle", label: "Lock Angular X", message: { type: "lock-angular-x" }, value: false },
    { type: "toggle", label: "Lock Angular Y", message: { type: "lock-angular-y" }, value: false },
    { type: "toggle", label: "Lock Angular Z", message: { type: "lock-angular-z" }, value: false },
    { type: "button", label: "Impulse (L)", message: { type: "impulse" } },
  ],
};

export const motionLocksSample = createGenericSample(
  "joints/motion-locks", "Joints / Motion Locks", spec,
  () => new Worker(new URL("./motion-locks.worker.ts", import.meta.url), { type: "module" }),
);
