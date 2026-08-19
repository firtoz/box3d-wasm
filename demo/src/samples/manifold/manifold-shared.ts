import * as THREE from "three";
import { type Vec3, type WorldTransform } from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { capsuleMesh } from "../shared";
import {
  MANIFOLD_CONTACT_STRIDE,
  MANIFOLD_COUNT_INDEX,
  MANIFOLD_FRAME_POS,
  MANIFOLD_FRAME_ROT,
  MANIFOLD_MAX_CONTACTS,
  MANIFOLD_NORMAL_INDEX,
  MANIFOLD_POINT_BASE,
  type ManifoldDraw,
  type ManifoldScene,
} from "./manifold-dump";

export {
  DEFAULT_MANIFOLD_A,
  DEFAULT_MANIFOLD_B,
  IDENTITY_XF,
  defaultManifoldA,
  dumpCheckpointManifold,
  dumpCreateManifold,
  dumpStepManifold,
  emptyLocalManifold,
  manifoldCamera,
  manifoldDumpJson,
} from "./manifold-dump";
export type { ManifoldDraw, ManifoldResources, ManifoldScene } from "./manifold-dump";

function applyWorldTransform(object: THREE.Object3D, transform: WorldTransform): void {
  object.position.set(transform.position[0], transform.position[1], transform.position[2]);
  object.quaternion.set(transform.rotation[0], transform.rotation[1], transform.rotation[2], transform.rotation[3]);
}

function transformPoint(transform: WorldTransform, local: Vec3): Vec3 {
  const q = new THREE.Quaternion(transform.rotation[0], transform.rotation[1], transform.rotation[2], transform.rotation[3]);
  const v = new THREE.Vector3(local[0], local[1], local[2]).applyQuaternion(q);
  return [transform.position[0] + v.x, transform.position[1] + v.y, transform.position[2] + v.z];
}

function poseDrawObject(object: THREE.Object3D, draw: ManifoldDraw, transform: WorldTransform): void {
  applyWorldTransform(object, transform);
  if (draw.kind === "sphere") {
    const local = new THREE.Vector3(draw.center[0], draw.center[1], draw.center[2]).applyQuaternion(object.quaternion);
    object.position.add(local);
  } else if (draw.kind === "capsule") {
    const mid = new THREE.Vector3(
      0.5 * (draw.center1[0] + draw.center2[0]),
      0.5 * (draw.center1[1] + draw.center2[1]),
      0.5 * (draw.center1[2] + draw.center2[2]),
    ).applyQuaternion(object.quaternion);
    object.position.add(mid);
  } else if (draw.kind === "box" && draw.localPosition !== undefined) {
    const local = new THREE.Vector3(...draw.localPosition).applyQuaternion(object.quaternion);
    object.position.add(local);
  }
}

function applyDrawOpacity(mesh: THREE.Mesh, opacity: number | undefined): void {
  if (opacity === undefined) return;
  const material = mesh.material as THREE.MeshStandardMaterial;
  material.transparent = true;
  material.opacity = opacity;
}

function addDrawMeshes(scene: THREE.Scene, draws: readonly ManifoldDraw[]): THREE.Object3D[] {
  const objects: THREE.Object3D[] = [];
  for (const draw of draws) {
    if (draw.kind === "sphere") {
      const mesh = new THREE.Mesh(
        new THREE.SphereGeometry(draw.radius, 24, 16),
        new THREE.MeshStandardMaterial({
          color: draw.color,
          roughness: 0.75,
        }),
      );
      applyDrawOpacity(mesh, draw.opacity);
      poseDrawObject(mesh, draw, draw.transform);
      mesh.visible = draw.followContactFrame !== true;
      scene.add(mesh);
      objects.push(mesh);
    } else if (draw.kind === "capsule") {
      const length = Math.hypot(
        draw.center2[0] - draw.center1[0],
        draw.center2[1] - draw.center1[1],
        draw.center2[2] - draw.center1[2],
      );
      const mesh = capsuleMesh(draw.radius, length, draw.color, 0.75, "x");
      applyDrawOpacity(mesh, draw.opacity);
      poseDrawObject(mesh, draw, draw.transform);
      mesh.visible = draw.followContactFrame !== true;
      scene.add(mesh);
      objects.push(mesh);
    } else if (draw.kind === "box") {
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(draw.size[0], draw.size[1], draw.size[2]),
        new THREE.MeshStandardMaterial({ color: draw.color, roughness: 0.75 }),
      );
      poseDrawObject(mesh, draw, draw.transform);
      mesh.visible = draw.followContactFrame !== true;
      scene.add(mesh);
      objects.push(mesh);
    } else {
      const p1 = transformPoint(draw.transform, draw.vertices[0]);
      const p2 = transformPoint(draw.transform, draw.vertices[1]);
      const p3 = transformPoint(draw.transform, draw.vertices[2]);
      const geometry = new THREE.BufferGeometry();
      geometry.setAttribute("position", new THREE.BufferAttribute(new Float32Array([...p1, ...p2, ...p2, ...p3, ...p3, ...p1]), 3));
      const lines = new THREE.LineSegments(geometry, new THREE.LineBasicMaterial({ color: draw.color, toneMapped: false }));
      scene.add(lines);
      objects.push(lines);
    }
  }
  return objects;
}

export function createManifoldHost(scene: ManifoldScene, createWorker: () => Worker) {
  const spec: RenderSpec = {
    groundSize: [20, 2, 20],
    groundKind: "none",
    bodies: [],
    camera: scene.camera,
    info: scene.info,
    overlay: (threeScene) => {
      const drawObjects = addDrawMeshes(threeScene, scene.draw);
      const axes = new THREE.AxesHelper(1);
      threeScene.add(axes);
      const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
      threeScene.add(grid);
      const contacts = Array.from({ length: MANIFOLD_MAX_CONTACTS }, () => ({
        point: createDebugPoint(threeScene, 0xfacc15, 10),
        normal: createDebugLine(threeScene, 0xffffff),
      }));
      for (const contact of contacts) {
        contact.point.visible = false;
        contact.normal.visible = false;
      }
      return {
        update({ workerState }) {
          const buffer = workerState?.extra?.manifold;
          if (!(buffer instanceof SharedArrayBuffer)) return;
          const values = new Float32Array(buffer);
          const frame: WorldTransform = {
            position: [values[MANIFOLD_FRAME_POS]!, values[MANIFOLD_FRAME_POS + 1]!, values[MANIFOLD_FRAME_POS + 2]!],
            rotation: [values[MANIFOLD_FRAME_ROT]!, values[MANIFOLD_FRAME_ROT + 1]!, values[MANIFOLD_FRAME_ROT + 2]!, values[MANIFOLD_FRAME_ROT + 3]!],
          };
          for (let i = 0; i < scene.draw.length; i++) {
            const draw = scene.draw[i]!;
            if (draw.followContactFrame !== true) continue;
            const object = drawObjects[i];
            if (object === undefined || draw.kind === "triangle") continue;
            poseDrawObject(object, draw, frame);
            object.visible = true;
          }
          const count = Math.min(MANIFOLD_MAX_CONTACTS, values[MANIFOLD_COUNT_INDEX] ?? 0);
          const nx = values[MANIFOLD_NORMAL_INDEX] ?? 0;
          const ny = values[MANIFOLD_NORMAL_INDEX + 1] ?? 0;
          const nz = values[MANIFOLD_NORMAL_INDEX + 2] ?? 0;
          for (let i = 0; i < MANIFOLD_MAX_CONTACTS; i++) {
            const visible = i < count;
            contacts[i]!.point.visible = visible;
            contacts[i]!.normal.visible = visible;
            if (!visible) continue;
            const base = MANIFOLD_POINT_BASE + i * MANIFOLD_CONTACT_STRIDE;
            const p: Vec3 = [values[base]!, values[base + 1]!, values[base + 2]!];
            updateDebugPoint(contacts[i]!.point, p);
            updateDebugLine(contacts[i]!.normal, p, [p[0] + 0.5 * nx, p[1] + 0.5 * ny, p[2] + 0.5 * nz]);
          }
        },
        dispose() {
          threeScene.remove(axes);
          axes.dispose();
          threeScene.remove(grid);
          grid.geometry.dispose();
          const gridMaterial = grid.material;
          if (Array.isArray(gridMaterial)) gridMaterial.forEach((material) => material.dispose());
          else gridMaterial.dispose();
          for (const object of drawObjects) {
            threeScene.remove(object);
            const mesh = object as THREE.Mesh;
            mesh.geometry?.dispose();
            const material = mesh.material;
            if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
            else material?.dispose();
          }
          for (const contact of contacts) {
            disposeDebugObject(threeScene, contact.point);
            disposeDebugObject(threeScene, contact.normal);
          }
        },
      };
    },
  };
  return createGenericSample(scene.id, scene.name, spec, createWorker);
}
