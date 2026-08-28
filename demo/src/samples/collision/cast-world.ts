import * as THREE from "three";
import { createGenericSample, meshFor } from "../generic-host";
import type { RenderBody, RenderSpec } from "../generic-host";
import {
  createDebugLine,
  createDebugPoint,
  disposeDebugObject,
  updateDebugLine,
  updateDebugPoint,
} from "../debug-overlay";
import { f32 } from "../f32";
import {
  CAST_KIND_CAPSULE,
  CAST_KIND_HEIGHT,
  CAST_KIND_HULL,
  CAST_KIND_MESH,
  CAST_KIND_SPHERE,
  CAST_WORLD_BODY_STRIDE,
  CAST_WORLD_HEADER_FLOATS,
  CAST_WORLD_HIT_CAPACITY,
  CAST_WORLD_HIT_STRIDE,
  CAST_WORLD_MAX,
  castWorldCamera,
  castWorldCapsuleLength,
  castWorldGroundSize,
  castWorldHeightFieldVisual,
  castWorldHullSize,
  castWorldMeshScale,
  castWorldSphereRadius,
  castWorldTorus,
} from "./cast-world-scene";

const half = castWorldGroundSize();
const HIT_COLORS = [0xff0000, 0x00ff00, 0x0000ff];
const MODE_LABELS = [
  "Cast mode: any - check for obstruction - unsorted",
  "Cast mode: closest - find closest shape along the cast",
  "Cast mode: multiple - gather multiple shapes - unsorted",
  "Cast mode: sorted - gather multiple shapes sorted by closeness",
];

function createHeightFieldMesh(): THREE.Group {
  const { rowCount, columnCount, scale, rowFrequency, columnFrequency } = castWorldHeightFieldVisual;
  const positions = new Float32Array(rowCount * columnCount * 3);
  let cursor = 0;
  for (let row = 0; row < rowCount; row++) {
    const rowHeight = f32(Math.sin(f32(2 * Math.PI * rowFrequency * row)));
    for (let column = 0; column < columnCount; column++) {
      const columnHeight = f32(Math.sin(f32(2 * Math.PI * columnFrequency * column)));
      positions[cursor++] = f32(column * scale[0]);
      positions[cursor++] = f32(scale[1] * rowHeight * columnHeight);
      positions[cursor++] = f32(row * scale[2]);
    }
  }
  const indices: number[] = [];
  for (let row = 0; row < rowCount - 1; row++) {
    for (let column = 0; column < columnCount - 1; column++) {
      const i1 = row * columnCount + column;
      const i2 = i1 + 1;
      const i3 = i2 + columnCount;
      const i4 = i3 - 1;
      indices.push(i1, i2, i3, i3, i4, i1);
    }
  }
  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(positions, 3));
  geom.setIndex(indices);
  geom.computeVertexNormals();
  const fill = new THREE.Mesh(
    geom,
    new THREE.MeshStandardMaterial({ color: 0x2a2a2a, roughness: 0.9, side: THREE.DoubleSide, flatShading: true }),
  );
  fill.receiveShadow = true;
  const root = new THREE.Group();
  root.add(fill);
  return root;
}

function meshForKind(kind: number): THREE.Object3D {
  if (kind === CAST_KIND_HEIGHT) return createHeightFieldMesh();
  const spec: RenderBody = kind === CAST_KIND_SPHERE
    ? { kind: "sphere", radius: castWorldSphereRadius, color: 0x888888, position: [0, 0, 0] }
    : kind === CAST_KIND_CAPSULE
      ? { kind: "capsule", radius: 0.8, length: castWorldCapsuleLength, axis: "x", color: 0x888888, position: [0, 0, 0] }
      : kind === CAST_KIND_HULL
        ? { kind: "box", size: castWorldHullSize, color: 0x888888, position: [0, 0, 0] }
        : {
            kind: "torus",
            radius: castWorldTorus.radius,
            tube: castWorldTorus.tube,
            radialSegments: castWorldTorus.radialSegments,
            tubularSegments: castWorldTorus.tubularSegments,
            color: 0x888888,
            position: [0, 0, 0],
            scale: [...castWorldMeshScale] as [number, number, number],
          };
  return meshFor(spec);
}

function disposeObject(scene: THREE.Scene, object: THREE.Object3D): void {
  scene.remove(object);
  object.traverse((child) => {
    const mesh = child as THREE.Mesh;
    if (mesh.geometry !== undefined) mesh.geometry.dispose();
    const material = (child as THREE.Mesh).material;
    if (material === undefined) return;
    if (Array.isArray(material)) material.forEach((entry) => entry.dispose());
    else material.dispose();
  });
}

function makeAabbHelper(): THREE.LineSegments {
  const geom = new THREE.BufferGeometry();
  geom.setAttribute("position", new THREE.BufferAttribute(new Float32Array(24 * 3), 3));
  return new THREE.LineSegments(geom, new THREE.LineBasicMaterial({ color: 0xffff00, toneMapped: false }));
}

function updateAabbHelper(helper: THREE.LineSegments, min: readonly number[], max: readonly number[]): void {
  const p = helper.geometry.getAttribute("position") as THREE.BufferAttribute;
  const corners: [number, number, number][] = [
    [min[0]!, min[1]!, min[2]!],
    [max[0]!, min[1]!, min[2]!],
    [max[0]!, max[1]!, min[2]!],
    [min[0]!, max[1]!, min[2]!],
    [min[0]!, min[1]!, max[2]!],
    [max[0]!, min[1]!, max[2]!],
    [max[0]!, max[1]!, max[2]!],
    [min[0]!, max[1]!, max[2]!],
  ];
  const edges: [number, number][] = [
    [0, 1], [1, 2], [2, 3], [3, 0],
    [4, 5], [5, 6], [6, 7], [7, 4],
    [0, 4], [1, 5], [2, 6], [3, 7],
  ];
  let i = 0;
  for (const [a, b] of edges) {
    const ca = corners[a]!;
    const cb = corners[b]!;
    p.setXYZ(i++, ca[0], ca[1], ca[2]);
    p.setXYZ(i++, cb[0], cb[1], cb[2]);
  }
  p.needsUpdate = true;
  helper.geometry.computeBoundingSphere();
}

const spec: RenderSpec = {
  groundKind: "none",
  groundSize: [2 * half[0], 2 * half[1], 2 * half[2]],
  bodies: [],
  camera: castWorldCamera,
  castPickRay: true,
  info: "Ctrl + left mouse to cast through cursor. Yellow boxes are ignored.",
  getInfo(workerState) {
    const buffer = workerState?.extra?.cast;
    if (!(buffer instanceof SharedArrayBuffer)) return undefined;
    const values = new Float32Array(buffer);
    const mode = values[6] | 0;
    const hitCount = values[10] | 0;
    const lines = [MODE_LABELS[mode] ?? MODE_LABELS[1]];
    for (let i = 0; i < hitCount; i++) {
      const o = CAST_WORLD_HEADER_FLOATS + i * CAST_WORLD_HIT_STRIDE;
      lines.push(`material = ${values[o + 7] | 0}, triangle = ${values[o + 8] | 0}`);
    }
    return lines.join("\n");
  },
  controls: [
    { type: "range", label: "Cast Type", message: { type: "set-type" }, min: 0, max: 3, step: 1, value: 0 },
    { type: "range", label: "Radius", message: { type: "set-radius" }, min: 0.1, max: 2, step: 0.1, value: 0.5 },
    { type: "range", label: "Mode", message: { type: "set-mode" }, min: 0, max: 3, step: 1, value: 1 },
    { type: "toggle", label: "Initial Overlap", message: { type: "set-overlap" }, value: false },
    { type: "button", label: "Spheres", message: { type: "spawn", kind: CAST_KIND_SPHERE, count: 10 } },
    { type: "button", label: "Capsules", message: { type: "spawn", kind: CAST_KIND_CAPSULE, count: 10 } },
    { type: "button", label: "Hulls", message: { type: "spawn", kind: CAST_KIND_HULL, count: 10 } },
    { type: "button", label: "Meshes", message: { type: "spawn", kind: CAST_KIND_MESH, count: 1 } },
    { type: "button", label: "Height Field", message: { type: "spawn", kind: CAST_KIND_HEIGHT, count: 1 } },
    { type: "button", label: "Destroy Shape", message: { type: "destroy" } },
  ],
  overlay: (scene) => {
    const grid = new THREE.GridHelper(10, 10, 0x4b5563, 0x4b5563);
    scene.add(grid);
    const axes = new THREE.AxesHelper(1);
    scene.add(axes);
    const rayLine = createDebugLine(scene, 0x00ffff);
    const originPoint = createDebugPoint(scene, 0x00ff00, 10);
    const hitPoints = HIT_COLORS.map((color) => createDebugPoint(scene, color, 10));
    const hitNormals = HIT_COLORS.map((color) => createDebugLine(scene, color));
    const aabbHelpers: THREE.LineSegments[] = [];
    for (let i = 0; i < CAST_WORLD_MAX; i++) {
      const helper = makeAabbHelper();
      helper.visible = false;
      scene.add(helper);
      aabbHelpers.push(helper);
    }
    const proxySphere = new THREE.Mesh(
      new THREE.SphereGeometry(1, 24, 16),
      new THREE.MeshStandardMaterial({ color: 0x888888, transparent: true, opacity: 0.5 }),
    );
    const proxyCapsule = meshFor({
      kind: "capsule",
      radius: 1,
      length: 1,
      axis: "y",
      color: 0x888888,
      position: [0, 0, 0],
      localPosition: [0, 0.5, 0],
    });
    const proxyBox = new THREE.Mesh(
      new THREE.BoxGeometry(2, 1, 0.5),
      new THREE.MeshStandardMaterial({ color: 0x888888, transparent: true, opacity: 0.5 }),
    );
    scene.add(proxySphere, proxyCapsule, proxyBox);
    proxySphere.visible = false;
    proxyCapsule.visible = false;
    proxyBox.visible = false;
    const spawned: THREE.Object3D[] = [];
    let lastKindKey = "";

    return {
      update({ workerState }) {
        const buffer = workerState?.extra?.cast;
        if (!(buffer instanceof SharedArrayBuffer) || workerState === null) return;
        const values = new Float32Array(buffer);
        const origin: [number, number, number] = [values[0]!, values[1]!, values[2]!];
        const translation: [number, number, number] = [values[3]!, values[4]!, values[5]!];
        const castType = values[7] | 0;
        const radius = values[8]!;
        const hitCount = values[10] | 0;
        const liveCount = values[11] | 0;
        const end: [number, number, number] = [
          origin[0] + translation[0],
          origin[1] + translation[1],
          origin[2] + translation[2],
        ];
        updateDebugLine(rayLine, origin, end);
        updateDebugPoint(originPoint, origin);

        const kindKey = `${liveCount}:${Array.from({ length: liveCount }, (_, i) => values[CAST_WORLD_HEADER_FLOATS + CAST_WORLD_HIT_CAPACITY * CAST_WORLD_HIT_STRIDE + i * CAST_WORLD_BODY_STRIDE] | 0).join(",")}`;
        if (kindKey !== lastKindKey) {
          lastKindKey = kindKey;
          for (const object of spawned) disposeObject(scene, object);
          spawned.length = 0;
          for (let i = 0; i < liveCount; i++) {
            const o = CAST_WORLD_HEADER_FLOATS + CAST_WORLD_HIT_CAPACITY * CAST_WORLD_HIT_STRIDE + i * CAST_WORLD_BODY_STRIDE;
            const object = meshForKind(values[o] | 0);
            scene.add(object);
            spawned.push(object);
          }
        }

        for (let i = 0; i < spawned.length; i++) {
          const object = spawned[i]!;
          const p = i * 3;
          const r = i * 4;
          object.position.set(workerState.positions[p]!, workerState.positions[p + 1]!, workerState.positions[p + 2]!);
          object.quaternion.set(
            workerState.rotations[r]!,
            workerState.rotations[r + 1]!,
            workerState.rotations[r + 2]!,
            workerState.rotations[r + 3]!,
          );
          const colorHex = workerState.colors[i]! & 0xffffff;
          object.traverse((child) => {
            const mesh = child as THREE.Mesh;
            const mat = mesh.material as THREE.MeshStandardMaterial | undefined;
            if (mat?.color !== undefined) mat.color.setHex(colorHex);
          });
        }

        for (let i = 0; i < CAST_WORLD_HIT_CAPACITY; i++) {
          const visible = i < hitCount;
          hitPoints[i]!.visible = visible;
          hitNormals[i]!.visible = visible;
          if (!visible) continue;
          const o = CAST_WORLD_HEADER_FLOATS + i * CAST_WORLD_HIT_STRIDE;
          const point: [number, number, number] = [values[o + 1]!, values[o + 2]!, values[o + 3]!];
          const normal: [number, number, number] = [values[o + 4]!, values[o + 5]!, values[o + 6]!];
          updateDebugPoint(hitPoints[i]!, point);
          updateDebugLine(hitNormals[i]!, point, [point[0] + 0.5 * normal[0], point[1] + 0.5 * normal[1], point[2] + 0.5 * normal[2]]);
        }

        const firstFraction = hitCount > 0 ? values[CAST_WORLD_HEADER_FLOATS]! : 1;
        const proxyPos: [number, number, number] = [
          origin[0] + firstFraction * translation[0],
          origin[1] + firstFraction * translation[1],
          origin[2] + firstFraction * translation[2],
        ];
        if (hitCount === 0) {
          proxyPos[0] = end[0];
          proxyPos[1] = end[1];
          proxyPos[2] = end[2];
        }
        proxySphere.visible = castType === 1;
        proxyCapsule.visible = castType === 2;
        proxyBox.visible = castType === 3;
        if (proxySphere.visible) {
          proxySphere.position.set(...proxyPos);
          proxySphere.scale.setScalar(radius);
          (proxySphere.material as THREE.MeshStandardMaterial).color.setHex(hitCount > 0 ? HIT_COLORS[0]! : 0x888888);
        }
        if (proxyCapsule.visible) {
          proxyCapsule.position.set(...proxyPos);
          proxyCapsule.scale.set(radius, 1, radius);
          (proxyCapsule.material as THREE.MeshStandardMaterial).color.setHex(hitCount > 0 ? HIT_COLORS[0]! : 0x888888);
        }
        if (proxyBox.visible) {
          proxyBox.position.set(...proxyPos);
          proxyBox.scale.set(radius, radius, radius);
          (proxyBox.material as THREE.MeshStandardMaterial).color.setHex(hitCount > 0 ? HIT_COLORS[0]! : 0x888888);
        }

        let aabbIndex = 0;
        for (let i = 0; i < liveCount; i++) {
          const o = CAST_WORLD_HEADER_FLOATS + CAST_WORLD_HIT_CAPACITY * CAST_WORLD_HIT_STRIDE + i * CAST_WORLD_BODY_STRIDE;
          if ((values[o + 1] | 0) !== 1) continue;
          const helper = aabbHelpers[aabbIndex++]!;
          helper.visible = true;
          updateAabbHelper(helper, [values[o + 2]!, values[o + 3]!, values[o + 4]!], [values[o + 5]!, values[o + 6]!, values[o + 7]!]);
        }
        for (let i = aabbIndex; i < CAST_WORLD_MAX; i++) aabbHelpers[i]!.visible = false;
      },
      dispose() {
        disposeDebugObject(scene, rayLine);
        disposeDebugObject(scene, originPoint);
        for (const point of hitPoints) disposeDebugObject(scene, point);
        for (const line of hitNormals) disposeDebugObject(scene, line);
        for (const helper of aabbHelpers) {
          scene.remove(helper);
          helper.geometry.dispose();
          (helper.material as THREE.Material).dispose();
        }
        disposeObject(scene, proxySphere);
        disposeObject(scene, proxyCapsule);
        disposeObject(scene, proxyBox);
        for (const object of spawned) disposeObject(scene, object);
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

export const castWorldSample = createGenericSample(
  "collision/cast-world",
  "Collision / Cast World",
  spec,
  () => new Worker(new URL("./cast-world.worker.ts", import.meta.url), { type: "module" }),
);
