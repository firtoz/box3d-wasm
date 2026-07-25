import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createDebugLine, disposeDebugObject, updateDebugLine } from "../debug-overlay";
import {
  PARALLEL_SPRING_BODY_ROTATION,
  PARALLEL_SPRING_DRAW_LENGTH,
  PARALLEL_SPRING_LOCAL_FRAME_A,
  parallelSpringBodies,
  parallelSpringCamera,
  parallelSpringGroundSize,
} from "./parallel-spring-scene";

const half = parallelSpringGroundSize();

const localFrameA = new THREE.Quaternion(
  PARALLEL_SPRING_LOCAL_FRAME_A[0],
  PARALLEL_SPRING_LOCAL_FRAME_A[1],
  PARALLEL_SPRING_LOCAL_FRAME_A[2],
  PARALLEL_SPRING_LOCAL_FRAME_A[3],
);
const bodyInit = new THREE.Quaternion(
  PARALLEL_SPRING_BODY_ROTATION[0],
  PARALLEL_SPRING_BODY_ROTATION[1],
  PARALLEL_SPRING_BODY_ROTATION[2],
  PARALLEL_SPRING_BODY_ROTATION[3],
);
/** `invMulQuat(bodyRot, frameA)` — local frame B on the dynamic box. */
const localFrameB = bodyInit.clone().invert().multiply(localFrameA);

const tmpQuat = new THREE.Quaternion();
const tmpAxis = new THREE.Vector3();
const start = [0, 0, 0] as [number, number, number];
const end = [0, 0, 0] as [number, number, number];

function updateFrameLine(
  line: THREE.Line,
  mesh: THREE.Object3D,
  localFrame: THREE.Quaternion,
): void {
  tmpQuat.copy(mesh.quaternion).multiply(localFrame);
  tmpAxis.set(0, 0, 1).applyQuaternion(tmpQuat).multiplyScalar(PARALLEL_SPRING_DRAW_LENGTH);
  start[0] = mesh.position.x;
  start[1] = mesh.position.y;
  start[2] = mesh.position.z;
  end[0] = start[0] + tmpAxis.x;
  end[1] = start[1] + tmpAxis.y;
  end[2] = start[2] + tmpAxis.z;
  updateDebugLine(line, start, end);
}

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: parallelSpringBodies,
  camera: parallelSpringCamera,
  info: "parallel spring joint (hertz=10, damping=0.7) — tilted box + arena walls",
  overlay: (scene) => {
    // Mirror `b3DrawParallelJoint`: green frame A Z, blue frame B Z (drawScale=2 → length 0.2).
    const frameALine = createDebugLine(scene, 0x22c55e);
    const frameBLine = createDebugLine(scene, 0x3b82f6);
    return {
      update({ bodies }) {
        const walls = bodies[0]?.mesh;
        const box = bodies[1]?.mesh;
        if (walls !== undefined) updateFrameLine(frameALine, walls, localFrameA);
        if (box !== undefined) updateFrameLine(frameBLine, box, localFrameB);
      },
      dispose() {
        disposeDebugObject(scene, frameALine);
        disposeDebugObject(scene, frameBLine);
      },
    };
  },
};

export const parallelSpringSample = createGenericSample(
  "joints/parallel-spring",
  "Joints / Parallel Spring",
  spec,
  () => new Worker(new URL("./parallel-spring.worker.ts", import.meta.url), { type: "module" }),
);
