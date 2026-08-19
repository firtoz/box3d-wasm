import * as THREE from "three";
import {
  type Box3DRuntime,
  type LocalManifold,
  type Vec3,
  type WorldTransform,
} from "box3d-wasm";
import { createGenericSample } from "../generic-host";
import type { RenderSpec } from "../generic-host";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { capsuleMesh } from "../shared";
import type { BodyId } from "box3d-wasm";
import {
  IDENTITY_XF,
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

const CONTACT_HEADER = 11;
const CONTACT_STRIDE = 4;
const MAX_CONTACTS = 64;
const FRAME_POS = 0;
const FRAME_ROT = 3;
const COUNT_INDEX = 7;
const NORMAL_INDEX = 8;
const POINT_BASE = 11;

function writeContactBuffer(runtime: Box3DRuntime, manifold: LocalManifold, frame: WorldTransform, out: Float32Array): void {
  out[FRAME_POS] = frame.position[0];
  out[FRAME_POS + 1] = frame.position[1];
  out[FRAME_POS + 2] = frame.position[2];
  out[FRAME_ROT] = frame.rotation[0];
  out[FRAME_ROT + 1] = frame.rotation[1];
  out[FRAME_ROT + 2] = frame.rotation[2];
  out[FRAME_ROT + 3] = frame.rotation[3];
  out[COUNT_INDEX] = manifold.pointCount;
  const worldNormal = runtime.rotateVector(frame.rotation, manifold.normal);
  out[NORMAL_INDEX] = worldNormal[0];
  out[NORMAL_INDEX + 1] = worldNormal[1];
  out[NORMAL_INDEX + 2] = worldNormal[2];
  for (let i = 0; i < manifold.pointCount && i < MAX_CONTACTS; i++) {
    const local = manifold.points[i]!;
    const rotated = runtime.rotateVector(frame.rotation, local.point);
    const base = POINT_BASE + i * CONTACT_STRIDE;
    out[base] = frame.position[0] + rotated[0];
    out[base + 1] = frame.position[1] + rotated[1];
    out[base + 2] = frame.position[2] + rotated[2];
    out[base + 3] = local.separation;
  }
}

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

function addDrawMeshes(scene: THREE.Scene, draws: readonly ManifoldDraw[]): THREE.Object3D[] {
  const objects: THREE.Object3D[] = [];
  for (const draw of draws) {
    if (draw.kind === "sphere") {
      const mesh = new THREE.Mesh(
        new THREE.SphereGeometry(draw.radius, 24, 16),
        new THREE.MeshStandardMaterial({
          color: draw.color,
          roughness: 0.75,
          transparent: draw.opacity !== undefined,
          opacity: draw.opacity ?? 1,
        }),
      );
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
      const mesh = capsuleMesh(draw.radius, length, draw.color, draw.opacity ?? 0.85, "x");
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
      const contacts = Array.from({ length: MAX_CONTACTS }, () => ({
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
            position: [values[FRAME_POS]!, values[FRAME_POS + 1]!, values[FRAME_POS + 2]!],
            rotation: [values[FRAME_ROT]!, values[FRAME_ROT + 1]!, values[FRAME_ROT + 2]!, values[FRAME_ROT + 3]!],
          };
          for (let i = 0; i < scene.draw.length; i++) {
            const draw = scene.draw[i]!;
            if (draw.followContactFrame !== true) continue;
            const object = drawObjects[i];
            if (object === undefined || draw.kind === "triangle") continue;
            poseDrawObject(object, draw, frame);
            object.visible = true;
          }
          const count = Math.min(MAX_CONTACTS, values[COUNT_INDEX] ?? 0);
          const nx = values[NORMAL_INDEX] ?? 0;
          const ny = values[NORMAL_INDEX + 1] ?? 0;
          const nz = values[NORMAL_INDEX + 2] ?? 0;
          for (let i = 0; i < MAX_CONTACTS; i++) {
            const visible = i < count;
            contacts[i]!.point.visible = visible;
            contacts[i]!.normal.visible = visible;
            if (!visible) continue;
            const base = POINT_BASE + i * CONTACT_STRIDE;
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

export class ManifoldWorker extends PhysicsWorkerBase {
  private collide: ((runtime: Box3DRuntime) => LocalManifold) | null = null;
  private disposeResources: (() => void) | null = null;
  private contactFrame: WorldTransform = IDENTITY_XF;
  private buffer: SharedArrayBuffer | null = null;
  private values: Float32Array | null = null;

  constructor(private readonly scene: ManifoldScene) {
    super();
  }

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<BodyId[]> {
    const resources = this.scene.create(this.runtime!);
    this.collide = resources.collide;
    this.disposeResources = resources.dispose;
    this.contactFrame = resources.contactFrame;
    this.buffer = new SharedArrayBuffer((CONTACT_HEADER + MAX_CONTACTS * CONTACT_STRIDE) * 4);
    this.values = new Float32Array(this.buffer);
    this.refresh();
    return [];
  }

  protected onBeforeDisposeWorld(): void {
    this.disposeResources?.();
    this.disposeResources = null;
    this.collide = null;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.buffer === null ? {} : { manifold: this.buffer };
  }

  protected stepPhysics(): void {
    this.totalSteps += 1;
    this.refresh();
  }

  private refresh(): void {
    if (this.runtime === null || this.collide === null || this.values === null) return;
    writeContactBuffer(this.runtime, this.collide(this.runtime), this.contactFrame, this.values);
  }
}
