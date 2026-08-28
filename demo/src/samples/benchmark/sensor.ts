import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  SENSOR_STATIC_COUNT,
  SENSOR_VISITOR_CAPACITY,
  forEachSensorStatic,
  sensorCamera,
  sensorGroundSize,
} from "./sensor-scene";

const half = sensorGroundSize();

export const benchmarkSensorSample = createShaderInstancedSample({
  id: "benchmark/sensor",
  name: "Benchmark / Sensor",
  createWorker: () => new Worker(new URL("./sensor.worker.ts", import.meta.url), { type: "module" }),
  instanceCount: SENSOR_STATIC_COUNT + SENSOR_VISITOR_CAPACITY,
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  camera: sensorCamera!,
  shape: { kind: "box", size: 1 },
  forEachInstance: (callback) => {
    forEachSensorStatic((position, color) => callback(position, color));
  },
  info: `${SENSOR_STATIC_COUNT} sensor cubes + falling visitors | custom filter row`,
});
