import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { createDebugLine, disposeDebugObject, updateDebugLine } from "../debug-overlay";
import {
  RAY_CURTAIN_HEADER_FLOATS,
  RAY_CURTAIN_RAY_COUNT,
  RAY_CURTAIN_RAY_STRIDE_FLOATS,
  rayCurtainBodies,
  rayCurtainCamera,
  rayCurtainGroundSize,
} from "./ray-curtain-scene";

const half = rayCurtainGroundSize();

function createDebugPoint(scene: THREE.Scene, color: number): THREE.Points {
  const geometry = new THREE.BufferGeometry();
  geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array(3), 3));
  const material = new THREE.PointsMaterial({ color, size: 4, sizeAttenuation: false, toneMapped: false });
  const points = new THREE.Points(geometry, material);
  scene.add(points);
  return points;
}

function updateDebugPoint(points: THREE.Points, position: readonly number[]): void {
  const attr = points.geometry.getAttribute("position") as THREE.BufferAttribute;
  attr.setXYZ(0, position[0]!, position[1]!, position[2]!);
  attr.needsUpdate = true;
  points.geometry.computeBoundingSphere();
}

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  // Upstream has no physics ground — DrawGroundGrid(10) only.
  groundKind: "none",
  bodies: rayCurtainBodies,
  camera: rayCurtainCamera,
  info: "kinematic targets + ray curtain (CastRayClosest)",
  overlay: (scene) => {
    const rays: THREE.Line[] = [];
    const normals: THREE.Line[] = [];
    const origins: THREE.Points[] = [];
    const ends: THREE.Points[] = [];
    for (let i = 0; i < RAY_CURTAIN_RAY_COUNT; i++) {
      rays.push(createDebugLine(scene, 0xfacc15));
      normals.push(createDebugLine(scene, 0x22c55e));
      normals[i]!.visible = false;
      origins.push(createDebugPoint(scene, 0x22c55e));
      ends.push(createDebugPoint(scene, 0xef4444));
    }

    // Upstream DrawGroundGrid(10) + RGB axes.
    const groundGrid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(groundGrid);
    const axes = new THREE.AxesHelper(0.4);
    scene.add(axes);

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.rays;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        for (let i = 0; i < RAY_CURTAIN_RAY_COUNT; i++) {
          const base = RAY_CURTAIN_HEADER_FLOATS + i * RAY_CURTAIN_RAY_STRIDE_FLOATS;
          const x = -8 + i * 0.1;
          const offset = values[0]!;
          const origin: [number, number, number] = [x, 8, offset];
          // Upstream always draws the full ray to y=0 (yellow), not clipped to the hit.
          const end: [number, number, number] = [x, 0, offset];
          updateDebugLine(rays[i]!, origin, end);
          updateDebugPoint(origins[i]!, origin);
          updateDebugPoint(ends[i]!, end);

          const u8 = new Uint8Array(values.buffer, values.byteOffset + base * 4, 4);
          const hit = u8[0] === 1;
          if (hit) {
            const hitPoint: [number, number, number] = [
              values[base + 2]!,
              values[base + 3]!,
              values[base + 4]!,
            ];
            const n: [number, number, number] = [
              hitPoint[0] + 0.5 * values[base + 5]!,
              hitPoint[1] + 0.5 * values[base + 6]!,
              hitPoint[2] + 0.5 * values[base + 7]!,
            ];
            updateDebugLine(normals[i]!, hitPoint, n);
            normals[i]!.visible = true;
          } else {
            normals[i]!.visible = false;
          }
        }
      },
      dispose() {
        for (const line of rays) disposeDebugObject(scene, line);
        for (const line of normals) disposeDebugObject(scene, line);
        for (const point of origins) disposeDebugObject(scene, point);
        for (const point of ends) disposeDebugObject(scene, point);
        scene.remove(groundGrid);
        groundGrid.geometry.dispose();
        const gridMats = groundGrid.material;
        if (Array.isArray(gridMats)) gridMats.forEach((m) => m.dispose());
        else gridMats.dispose();
        scene.remove(axes);
        axes.dispose();
      },
    };
  },
};

export const rayCurtainSample = createGenericSample(
  "collision/ray-curtain",
  "Collision / Ray Curtain",
  spec,
  () => new Worker(new URL("./ray-curtain.worker.ts", import.meta.url), { type: "module" }),
);
