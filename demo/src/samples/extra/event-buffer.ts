import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { extraEventBufferBodies, extraEventBufferCamera, extraEventBufferGroundSize } from "./event-buffer-scene";

const half = extraEventBufferGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: extraEventBufferBodies,
  camera: extraEventBufferCamera,
  info: "Zero-GC fillContactEvents / fillSensorEvents / fillBodyContactData / fillBodyContactManifolds",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.eventBufferHud;
    if (!(buffer instanceof SharedArrayBuffer)) return spec.info;
    const v = new Int32Array(buffer);
    return [
      `contact begin/end/hit = ${v[0]}/${v[1]}/${v[2]}`,
      `sensor begin/end = ${v[3]}/${v[4]}`,
      `visitor body contacts = ${v[5]}`,
      `packed pair slots (heap view) = ${v[6]}`,
      `manifolds contacts/points = ${v[7]}/${v[8]}  max normal impulse = ${((v[9] ?? 0) / 1000).toFixed(3)}`,
    ].join("\n");
  },
};

export const extraEventBufferSample = createGenericSample(
  "extra/event-buffer",
  "Extra / Event Buffer",
  spec,
  () => new Worker(new URL("./event-buffer.worker.ts", import.meta.url), { type: "module" }),
);
