import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createDebugLine, createWireSphere, disposeDebugObject, updateDebugLine } from "../debug-overlay";
import { WEEBLE_EXPLOSION_DEFAULT, WEEBLE_EXPLOSION_MAX, WEEBLE_EXPLOSION_MIN, weebleBodies, weebleCamera, weebleGroundSize } from "./weeble-scene";

const half = weebleGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: weebleBodies,
  camera: weebleCamera,
  info: "Weeble wobbles but doesn't fall down.",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.debug;
    if (!(buffer instanceof SharedArrayBuffer)) return undefined;
    const values = new Float32Array(buffer);
    const localSpeed = Math.hypot(values[3], values[4], values[5]);
    const worldSpeed = Math.hypot(values[6], values[7], values[8]);
    return `Weeble wobbles but doesn't fall down. | magnitude ${values[9].toFixed(0)} | local point speed ${localSpeed.toFixed(2)} | world point speed ${worldSpeed.toFixed(2)}`;
  },
  controls: [
    { type: "button", label: "Teleport", message: { type: "teleport" } },
    { type: "button", label: "Explode", message: { type: "explode" } },
    { type: "range", label: "Magnitude", message: { type: "set-explosion-magnitude" }, min: WEEBLE_EXPLOSION_MIN, max: WEEBLE_EXPLOSION_MAX, step: 1000, value: WEEBLE_EXPLOSION_DEFAULT },
  ],
  overlay: (scene) => {
    const sphere = createWireSphere(scene, 8, 0x38bdf8);
    sphere.position.set(0, -0.1, 0);
    const localLine = createDebugLine(scene, 0xef4444);
    const worldLine = createDebugLine(scene, 0x22c55e);
    const start = [0, 0, 0] as [number, number, number];
    const localEnd = [0, 0, 0] as [number, number, number];
    const worldEnd = [0, 0, 0] as [number, number, number];
    const offset = [0.05, 0, 0] as const;
    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.debug;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        start[0] = values[0];
        start[1] = values[1];
        start[2] = values[2];
        localEnd[0] = values[0] + values[3];
        localEnd[1] = values[1] + values[4];
        localEnd[2] = values[2] + values[5];
        worldEnd[0] = values[0] + offset[0] + values[6];
        worldEnd[1] = values[1] + offset[1] + values[7];
        worldEnd[2] = values[2] + offset[2] + values[8];
        updateDebugLine(localLine, start, localEnd);
        updateDebugLine(worldLine, [start[0] + offset[0], start[1] + offset[1], start[2] + offset[2]], worldEnd);
      },
      dispose() {
        disposeDebugObject(scene, sphere);
        disposeDebugObject(scene, localLine);
        disposeDebugObject(scene, worldLine);
      },
    };
  },
};

export const weebleSample = createGenericSample(
  "bodies/weeble", "Bodies / Weeble", spec,
  () => new Worker(new URL("./weeble.worker.ts", import.meta.url), { type: "module" }),
);
