import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createDebugLine, disposeDebugObject, updateDebugLine } from "../debug-overlay";
import {
  createGyroscopicPrecessionBodies,
  gyroscopicPrecessionCamera,
  gyroscopicPrecessionGroundSize,
} from "./gyroscopic-precession-scene";

const half = gyroscopicPrecessionGroundSize();
const AXIS_LENGTH = 5;
const tmpAxis = new THREE.Vector3();
const start = [0, 0, 0] as [number, number, number];
const end = [0, 0, 0] as [number, number, number];

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: createGyroscopicPrecessionBodies(),
  camera: gyroscopicPrecessionCamera,
  info: "8×8 spinning tops — allowFastRotation + gyroscopic precession",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.debug;
    if (!(buffer instanceof SharedArrayBuffer)) return undefined;
    const values = new Float32Array(buffer);
    if (values[0] === 0) return "top is sleeping";
    return `spin ${values[1]!.toFixed(1)} rad/s, tilt ${values[2]!.toFixed(1)} deg`;
  },
  overlay: (scene) => {
    // Mirror C++ yellow tip→5*localY axis on the measured top (body 0).
    const axisLine = createDebugLine(scene, 0xfacc15);
    return {
      update({ bodies, workerState }) {
        const top = bodies[0]?.mesh;
        if (top === undefined) return;
        const buffer = workerState?.extra?.debug;
        if (buffer instanceof SharedArrayBuffer) {
          const values = new Float32Array(buffer);
          if (values[0] === 0) {
            axisLine.visible = false;
            return;
          }
        }
        axisLine.visible = true;
        tmpAxis.set(0, 1, 0).applyQuaternion(top.quaternion).multiplyScalar(AXIS_LENGTH);
        start[0] = top.position.x;
        start[1] = top.position.y;
        start[2] = top.position.z;
        end[0] = start[0] + tmpAxis.x;
        end[1] = start[1] + tmpAxis.y;
        end[2] = start[2] + tmpAxis.z;
        updateDebugLine(axisLine, start, end);
      },
      dispose() {
        disposeDebugObject(scene, axisLine);
      },
    };
  },
};

export const gyroscopicPrecessionSample = createGenericSample(
  "bodies/gyroscopic-precession",
  "Bodies / Gyroscopic Precession",
  spec,
  () => new Worker(new URL("./gyroscopic-precession.worker.ts", import.meta.url), { type: "module" }),
);
