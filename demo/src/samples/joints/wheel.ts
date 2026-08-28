import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { wheelBodies, wheelCamera, wheelGroundSize } from "./wheel-scene";

const half = wheelGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: wheelBodies,
  camera: wheelCamera,
  info: "wheel joint — suspension / spin / steering (defaults match C++ off)",
  controls: [
    { type: "toggle", label: "Suspension", message: { type: "enable-suspension" }, value: false },
    { type: "toggle", label: "Susp Limit", message: { type: "enable-suspension-limit" }, value: false },
    { type: "range", label: "Susp Min", message: { type: "set-suspension-min" }, min: -5, max: 5, step: 0.1, value: -1 },
    { type: "range", label: "Susp Max", message: { type: "set-suspension-max" }, min: -5, max: 5, step: 0.1, value: 1 },
    { type: "range", label: "Susp Hertz", message: { type: "set-suspension-hertz" }, min: 0, max: 10, step: 0.1, value: 2 },
    { type: "range", label: "Susp Damping", message: { type: "set-suspension-damping" }, min: 0, max: 2, step: 0.1, value: 0.7 },
    { type: "toggle", label: "Spin Motor", message: { type: "enable-spin-motor" }, value: false },
    { type: "range", label: "Max Spin Torque", message: { type: "set-max-spin-torque" }, min: 0, max: 100, step: 1, value: 20 },
    { type: "range", label: "Spin Speed", message: { type: "set-spin-speed" }, min: -50, max: 50, step: 1, value: 0 },
    { type: "toggle", label: "Steering", message: { type: "enable-steering" }, value: false },
    { type: "toggle", label: "Steer Limit", message: { type: "enable-steering-limit" }, value: false },
    { type: "range", label: "Steer Hertz", message: { type: "set-steering-hertz" }, min: 0, max: 20, step: 0.1, value: 1 },
    { type: "range", label: "Steer Damping", message: { type: "set-steering-damping" }, min: 0, max: 2, step: 0.1, value: 0.7 },
    { type: "range", label: "Steer Min Deg", message: { type: "set-steering-min-deg" }, min: -90, max: 0, step: 1, value: -45 },
    { type: "range", label: "Steer Max Deg", message: { type: "set-steering-max-deg" }, min: 0, max: 90, step: 1, value: 45 },
  ],
};

export const wheelSample = createGenericSample(
  "joints/wheel",
  "Joints / Wheel",
  spec,
  () => new Worker(new URL("./wheel.worker.ts", import.meta.url), { type: "module" }),
);
