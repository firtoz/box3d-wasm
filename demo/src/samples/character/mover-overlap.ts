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
  MOVER_OVERLAP_HEADER_FLOATS,
  MOVER_OVERLAP_STRIDE_FLOATS,
  moverOverlapBodies,
  moverOverlapCamera,
  moverOverlapCapsule,
  moverOverlapGroundSize,
} from "./mover-overlap-scene";

const half = moverOverlapGroundSize();

function addMoverCapsule(scene: THREE.Scene, color: number): THREE.Mesh {
  const length = Math.hypot(
    moverOverlapCapsule.center2[0] - moverOverlapCapsule.center1[0],
    moverOverlapCapsule.center2[1] - moverOverlapCapsule.center1[1],
    moverOverlapCapsule.center2[2] - moverOverlapCapsule.center1[2],
  );
  const mesh = capsuleMesh(moverOverlapCapsule.radius, length, color, 0.75, "y");
  scene.add(mesh);
  return mesh;
}

const spec: RenderSpec = {
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  groundKind: "none",
  bodies: moverOverlapBodies,
  camera: moverOverlapCamera,
  info: "drag the capsule with the left mouse to push it into the shapes",
  getInfo: (workerState) => {
    const buffer = workerState?.extra?.planes;
    if (!(buffer instanceof SharedArrayBuffer)) return undefined;
    const values = new Float32Array(buffer);
    return `yellow = queried pose, cyan = solved push-out | planes: ${values[3] | 0}   degenerate normals: ${values[4] | 0}`;
  },
  overlay: (scene) => {
    const grid = new THREE.GridHelper(12, 12, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const queried = addMoverCapsule(scene, 0xfacc15);
    const solved = addMoverCapsule(scene, 0x22d3ee);
    const points: THREE.Points[] = [];
    const lines: THREE.Line[] = [];
    for (let i = 0; i < 8; i++) {
      points.push(createDebugPoint(scene, 0x84cc16));
      lines.push(createDebugLine(scene, 0x84cc16));
    }

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.planes;
        if (!(buffer instanceof SharedArrayBuffer)) return;
        const values = new Float32Array(buffer);
        queried.position.set(values[0]!, values[1]!, values[2]!);
        solved.position.set(values[5]!, values[6]!, values[7]!);
        const count = Math.min(points.length, values[3] | 0);
        for (let i = 0; i < points.length; i++) {
          const visible = i < count;
          points[i]!.visible = visible;
          lines[i]!.visible = visible;
          if (!visible) continue;
          const base = MOVER_OVERLAP_HEADER_FLOATS + i * MOVER_OVERLAP_STRIDE_FLOATS;
          const origin: [number, number, number] = [values[0]!, values[1]!, values[2]!];
          const point: [number, number, number] = [
            origin[0] + values[base + 4]!,
            origin[1] + values[base + 5]!,
            origin[2] + values[base + 6]!,
          ];
          const normal: [number, number, number] = [values[base]!, values[base + 1]!, values[base + 2]!];
          const valid = values[base + 7] === 1;
          const color = valid ? 0x84cc16 : 0xef4444;
          (points[i]!.material as THREE.PointsMaterial).color.setHex(color);
          (lines[i]!.material as THREE.LineBasicMaterial).color.setHex(color);
          updateDebugPoint(points[i]!, point);
          updateDebugLine(lines[i]!, point, [
            point[0] + 0.5 * normal[0],
            point[1] + 0.5 * normal[1],
            point[2] + 0.5 * normal[2],
          ]);
        }
      },
      dispose() {
        for (const mesh of [queried, solved]) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          const material = mesh.material;
          if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
          else material.dispose();
        }
        for (const point of points) disposeDebugObject(scene, point);
        for (const line of lines) disposeDebugObject(scene, line);
        scene.remove(grid);
        grid.geometry.dispose();
        const gridMaterial = grid.material;
        if (Array.isArray(gridMaterial)) gridMaterial.forEach((entry) => entry.dispose());
        else gridMaterial.dispose();
      },
    };
  },
};

export const moverOverlapSample = createGenericSample(
  "character/mover-overlap",
  "Character / MoverOverlap",
  spec,
  () => new Worker(new URL("./mover-overlap.worker.ts", import.meta.url), { type: "module" }),
);
