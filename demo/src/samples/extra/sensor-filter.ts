import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { extraSensorFilterBodies, extraSensorFilterCamera, extraSensorFilterGroundSize } from "./sensor-filter-scene";

const half = extraSensorFilterGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: extraSensorFilterBodies,
  camera: extraSensorFilterCamera,
  info: "Cube turns lime on sensor overlap. Magenta is custom-filtered (skipped until toggle off).",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.sensorFilterHud;
    if (!(buffer instanceof SharedArrayBuffer)) return spec.info;
    const v = new Int32Array(buffer);
    return `sensor begin/end = ${v[0]}/${v[1]}\nfilter ${v[2] ? "on (skip magenta row)" : "off"}`;
  },
  controls: [
    { type: "toggle", label: "Custom filter", message: { type: "setFilter" }, value: true },
  ],
};

export const extraSensorFilterSample = createGenericSample(
  "extra/sensor-filter",
  "Extra / Sensor Filter",
  spec,
  () => new Worker(new URL("./sensor-filter.worker.ts", import.meta.url), { type: "module" }),
);
