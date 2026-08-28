import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { extraWorldKnobsBodies, extraWorldKnobsCamera, extraWorldKnobsGroundSize } from "./world-knobs-scene";

const half = extraWorldKnobsGroundSize();
const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: extraWorldKnobsBodies,
  camera: extraWorldKnobsCamera,
  info: "Live SetGravity / EnableSpeculative plus world getters, OverlapShape, rebuild static tree, profile, stall, workers",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.worldKnobsHud;
    if (!(buffer instanceof SharedArrayBuffer)) return spec.info;
    const v = new Float32Array(buffer);
    const profile = v[14] === 0 ? "off" : v[14] === 1 ? "coarse" : "full";
    return [
      `gravity = (${v[0]?.toFixed(2)}, ${v[1]?.toFixed(2)}, ${v[2]?.toFixed(2)})`,
      `restitution threshold = ${v[3]?.toFixed(3)}`,
      `hit event threshold = ${v[4]?.toFixed(3)}`,
      `max linear speed = ${v[5]?.toFixed(1)}`,
      `contact recycle distance = ${v[6]?.toFixed(3)}`,
      `overlap sphere count @ (0,2,0) r=2 = ${v[7] ?? 0}`,
      `static / dynamic tree height = ${v[8] ?? 0} / ${v[9] ?? 0}`,
      `bodies / contacts = ${v[10] ?? 0} / ${v[11] ?? 0}`,
      `workers = ${v[12] ?? 0}  stall = ${v[13]?.toFixed(3)}s  profile = ${profile}`,
    ].join("\n");
  },
  controls: [
    { type: "range", label: "gravity Y", message: { type: "setGravityY" }, min: -20, max: 5, step: 0.5, value: -10 },
    { type: "toggle", label: "Speculative", message: { type: "setSpeculative" }, value: true },
    { type: "button", label: "Rebuild static tree", message: { type: "rebuildStaticTree" } },
    { type: "range", label: "profile level", message: { type: "setProfileLevel" }, min: 0, max: 2, step: 1, value: 2 },
  ],
};

export const extraWorldKnobsSample = createGenericSample(
  "extra/world-knobs",
  "Extra / World Knobs",
  spec,
  () => new Worker(new URL("./world-knobs.worker.ts", import.meta.url), { type: "module" }),
);
