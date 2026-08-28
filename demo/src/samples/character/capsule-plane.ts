import * as THREE from "three";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { capsuleMesh } from "../shared";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import {
  CAPSULE_PLANE_CAPACITY,
  CAPSULE_PLANE_HEADER_FLOATS,
  CAPSULE_PLANE_STRIDE_FLOATS,
  capsulePlaneBodies,
  capsulePlaneCamera,
  capsulePlaneCapsule,
  capsulePlaneGroundSize,
} from "./capsule-plane-scene";

const half = capsulePlaneGroundSize();

function addMoverCapsule(scene: THREE.Scene): THREE.Mesh {
  const length = Math.hypot(
    capsulePlaneCapsule.center2[0] - capsulePlaneCapsule.center1[0],
    capsulePlaneCapsule.center2[1] - capsulePlaneCapsule.center1[1],
    capsulePlaneCapsule.center2[2] - capsulePlaneCapsule.center1[2],
  );
  const mesh = capsuleMesh(capsulePlaneCapsule.radius, length, 0x22c55e, 0.75, "y");
  scene.add(mesh);
  return mesh;
}

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: capsulePlaneBodies,
  camera: capsulePlaneCamera,
  info: "CollideMover capsule vs static box (Solve applies b3SolvePlanes)",
  controls: [
    { type: "button", label: "Solve", message: { type: "solve" } },
  ],
  overlay: (scene) => {
    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const axes = new THREE.AxesHelper(2);
    scene.add(axes);
    const capsule = addMoverCapsule(scene);
    const points: THREE.Points[] = [];
    const lines: THREE.Line[] = [];
    for (let i = 0; i < CAPSULE_PLANE_CAPACITY; i++) {
      points.push(createDebugPoint(scene, 0xfacc15));
      lines.push(createDebugLine(scene, 0xfacc15));
    }

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.planes;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        const origin: [number, number, number] = [values[0]!, values[1]!, values[2]!];
        capsule.position.set(origin[0], origin[1], origin[2]);
        const count = values[3] | 0;
        for (let i = 0; i < CAPSULE_PLANE_CAPACITY; i++) {
          const visible = i < count;
          points[i]!.visible = visible;
          lines[i]!.visible = visible;
          if (!visible) continue;
          const base = CAPSULE_PLANE_HEADER_FLOATS + i * CAPSULE_PLANE_STRIDE_FLOATS;
          const nx = values[base]!;
          const ny = values[base + 1]!;
          const nz = values[base + 2]!;
          const offset = values[base + 3]!;
          const p1: [number, number, number] = [
            origin[0] + (offset - capsulePlaneCapsule.radius) * nx,
            origin[1] + (offset - capsulePlaneCapsule.radius) * ny,
            origin[2] + (offset - capsulePlaneCapsule.radius) * nz,
          ];
          const p2: [number, number, number] = [p1[0] + 0.1 * nx, p1[1] + 0.1 * ny, p1[2] + 0.1 * nz];
          updateDebugPoint(points[i]!, p1);
          updateDebugLine(lines[i]!, p1, p2);
        }
      },
      dispose() {
        scene.remove(capsule);
        capsule.geometry.dispose();
        const material = capsule.material;
        if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
        else material.dispose();
        for (const point of points) disposeDebugObject(scene, point);
        for (const line of lines) disposeDebugObject(scene, line);
        scene.remove(grid);
        grid.geometry.dispose();
        const gridMaterial = grid.material;
        if (Array.isArray(gridMaterial)) gridMaterial.forEach((entry) => entry.dispose());
        else gridMaterial.dispose();
        scene.remove(axes);
        axes.dispose();
      },
    };
  },
};

export const capsulePlaneSample = createGenericSample(
  "character/capsule-plane",
  "Character / CapsulePlane",
  spec,
  () => new Worker(new URL("./capsule-plane.worker.ts", import.meta.url), { type: "module" }),
);
