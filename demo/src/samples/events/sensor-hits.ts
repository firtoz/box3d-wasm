import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  SENSOR_HITS_LAUNCH_SPEED,
  sensorHitsBodies,
  sensorHitsCamera,
  sensorHitsGroundSize,
} from "./sensor-hits-scene";

const half = sensorHitsGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: sensorHitsBodies,
  camera: sensorHitsCamera,
  info: `static/kinematic/dynamic sensors + launch sphere (dump speed ${SENSOR_HITS_LAUNCH_SPEED})`,
};

export const sensorHitsSample = createGenericSample(
  "events/sensor-hits",
  "Events / Sensor Hits",
  spec,
  () => new Worker(new URL("./sensor-hits.worker.ts", import.meta.url), { type: "module" }),
);
