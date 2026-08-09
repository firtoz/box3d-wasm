import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import {
  CAPSULE_CAST_RAY_HEADER_FLOATS,
  capsuleCastRayBodies,
  capsuleCastRayCamera,
  capsuleCastRayGroundSize,
  capsuleCastRayOrigin,
  capsuleCastRayTranslation,
} from "./capsule-cast-ray-scene";

const half = capsuleCastRayGroundSize();

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: capsuleCastRayBodies,
  camera: capsuleCastRayCamera,
  info: "Body_CastRay against a kinematic Y-axis capsule",
  overlay: (scene) => {
    const ray = createDebugLine(scene, 0x9ca3af);
    const origin = createDebugPoint(scene, 0x22c55e);
    const end = createDebugPoint(scene, 0xef4444);
    const hit = createDebugPoint(scene, 0xf97316);
    hit.visible = false;

    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const axes = new THREE.AxesHelper(0.4);
    scene.add(axes);

    const rayEnd: [number, number, number] = [
      capsuleCastRayOrigin[0] + capsuleCastRayTranslation[0],
      capsuleCastRayOrigin[1] + capsuleCastRayTranslation[1],
      capsuleCastRayOrigin[2] + capsuleCastRayTranslation[2],
    ];
    updateDebugLine(ray, capsuleCastRayOrigin, rayEnd);
    updateDebugPoint(origin, capsuleCastRayOrigin);
    updateDebugPoint(end, rayEnd);

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.rays;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        const base = CAPSULE_CAST_RAY_HEADER_FLOATS;
        const u8 = new Uint8Array(values.buffer, values.byteOffset + base * 4, 4);
        if (u8[0] === 1) {
          updateDebugPoint(hit, [values[base + 2]!, values[base + 3]!, values[base + 4]!]);
          hit.visible = true;
        } else {
          hit.visible = false;
        }
      },
      dispose() {
        disposeDebugObject(scene, ray);
        disposeDebugObject(scene, origin);
        disposeDebugObject(scene, end);
        disposeDebugObject(scene, hit);
        scene.remove(grid);
        grid.geometry.dispose();
        const gridMaterial = grid.material;
        if (Array.isArray(gridMaterial)) gridMaterial.forEach((material) => material.dispose());
        else gridMaterial.dispose();
        scene.remove(axes);
        axes.dispose();
      },
    };
  },
};

export const capsuleCastRaySample = createGenericSample(
  "collision/capsule-cast-ray",
  "Collision / Capsule Cast Ray",
  spec,
  () => new Worker(new URL("./capsule-cast-ray.worker.ts", import.meta.url), { type: "module" }),
);
