import { createShaderInstancedSample } from "../shader-instanced-host";
import {
  forEachJointGridSphere,
  JOINT_GRID_SPHERE_COUNT,
  JOINT_GRID_SPHERE_RADIUS,
  jointGridCamera,
  jointGridGroundSize,
} from "./joint-grid-scene";

const half = jointGridGroundSize();

export const jointGridSample = createShaderInstancedSample({
  id: "benchmark/joint-grid",
  name: "Benchmark / Joint Grid",
  createWorker: () => new Worker(new URL("./joint-grid.worker.ts", import.meta.url), { type: "module" }),
  instanceCount: JOINT_GRID_SPHERE_COUNT,
  // Upstream CreateJointGrid has no ground; the grid hangs downward from y=0.
  // A decorative plane at y=-1 made spheres look like they fell through the floor.
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  camera: jointGridCamera!,
  shape: { kind: "sphere", radius: JOINT_GRID_SPHERE_RADIUS },
  forEachInstance: (callback) => {
    forEachJointGridSphere((position, color) => callback(position, color));
  },
  info: ({ workerCount, colorMode }) =>
    `${JOINT_GRID_SPHERE_COUNT} spheres (100×100) | shader render | sleep off | ${workerCount} workers | ${colorMode} colors (C)`,
});
