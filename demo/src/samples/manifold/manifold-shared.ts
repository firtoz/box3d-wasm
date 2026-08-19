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

const CONTACT_HEADER = 4;
const CONTACT_STRIDE = 4;
const MAX_CONTACTS = 64;

function writeContactBuffer(runtime: Box3DRuntime, manifold: LocalManifold, frame: WorldTransform, out: Float32Array): void {
  out[0] = manifold.pointCount;
  const worldNormal = runtime.rotateVector(frame.rotation, manifold.normal);
  out[1] = worldNormal[0];
  out[2] = worldNormal[1];
  out[3] = worldNormal[2];
  for (let i = 0; i < manifold.pointCount && i < MAX_CONTACTS; i++) {
    const local = manifold.points[i]!;
    const rotated = runtime.rotateVector(frame.rotation, local.point);
    const base = CONTACT_HEADER + i * CONTACT_STRIDE;
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
      applyWorldTransform(mesh, draw.transform);
      const local = new THREE.Vector3(draw.center[0], draw.center[1], draw.center[2]).applyQuaternion(mesh.quaternion);
      mesh.position.add(local);
      scene.add(mesh);
      objects.push(mesh);
    } else if (draw.kind === "capsule") {
      const length = Math.hypot(
        draw.center2[0] - draw.center1[0],
        draw.center2[1] - draw.center1[1],
        draw.center2[2] - draw.center1[2],
      );
      const mesh = capsuleMesh(draw.radius, length, draw.color, draw.opacity ?? 0.85, "x");
      applyWorldTransform(mesh, draw.transform);
      const mid = new THREE.Vector3(
        0.5 * (draw.center1[0] + draw.center2[0]),
        0.5 * (draw.center1[1] + draw.center2[1]),
        0.5 * (draw.center1[2] + draw.center2[2]),
      ).applyQuaternion(mesh.quaternion);
      mesh.position.add(mid);
      scene.add(mesh);
      objects.push(mesh);
    } else if (draw.kind === "box") {
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(draw.size[0], draw.size[1], draw.size[2]),
        new THREE.MeshStandardMaterial({ color: draw.color, roughness: 0.75 }),
      );
      applyWorldTransform(mesh, draw.transform);
      if (draw.localPosition !== undefined) {
        const local = new THREE.Vector3(...draw.localPosition).applyQuaternion(mesh.quaternion);
        mesh.position.add(local);
      }
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
          const count = Math.min(MAX_CONTACTS, values[0] ?? 0);
          const nx = values[1] ?? 0;
          const ny = values[2] ?? 0;
          const nz = values[3] ?? 0;
          for (let i = 0; i < MAX_CONTACTS; i++) {
            const visible = i < count;
            contacts[i]!.point.visible = visible;
            contacts[i]!.normal.visible = visible;
            if (!visible) continue;
            const base = CONTACT_HEADER + i * CONTACT_STRIDE;
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
    this.refresh();
  }

  private refresh(): void {
    if (this.runtime === null || this.collide === null || this.values === null) return;
    writeContactBuffer(this.runtime, this.collide(this.runtime), this.contactFrame, this.values);
  }
}
