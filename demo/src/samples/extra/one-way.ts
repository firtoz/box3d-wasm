import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { extraOneWayBodies, extraOneWayCamera, extraOneWayGroundSize } from "./one-way-scene";

const half = extraOneWayGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: extraOneWayBodies,
  camera: extraOneWayCamera,
  info: "Green platform is one-way via SetPreSolveCallback. Blue drops from above and lands. Orange launches from below and passes through.",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.oneWayHud;
    if (!(buffer instanceof SharedArrayBuffer)) return spec.info;
    const v = new Int32Array(buffer);
    return `pre-solve keep = ${v[0] ?? 0}\npre-solve skip = ${v[1] ?? 0}`;
  },
  controls: [
    { type: "button", label: "Drop from above", message: { type: "drop" } },
    { type: "button", label: "Launch from below", message: { type: "launch" } },
  ],
};

export const extraOneWaySample = createGenericSample(
  "extra/one-way",
  "Extra / One-Way Platforms",
  spec,
  () => new Worker(new URL("./one-way.worker.ts", import.meta.url), { type: "module" }),
);
