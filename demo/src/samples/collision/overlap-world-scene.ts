import {
  B3_AXIS_X,
  B3_AXIS_Z,
  B3_PI,
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type HeightFieldHandle,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const BOX_HALF: Vec3 = [0.6, 0.6, 0.6];
const SPHERE_RADIUS = 0.8;
const CAPSULE: { center1: Vec3; center2: Vec3; radius: number } = {
  center1: [-0.5, 0, 0],
  center2: [0.5, 0, 0],
  radius: 0.5,
};
const MESH_SCALE: Vec3 = [-0.5, 1.5, -1.0];

const HF_ROW_COUNT = 10;
const HF_COLUMN_COUNT = 10;
const HF_SCALE: Vec3 = [0.2, 0.2, 0.2];
const HF_ROW_FREQUENCY = 0.03;
const HF_COLUMN_FREQUENCY = 0.09;

const ROW_SHAPES = [
  {
    x: -6,
    yBase: 3,
    rotation: (runtime: Box3DRuntime) => runtime.makeQuatFromAxisAngle(B3_AXIS_X, 0.5 * B3_PI),
    kind: "sphere" as const,
  },
  {
    x: -3,
    yBase: 3,
    rotation: (runtime: Box3DRuntime) => runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.25 * B3_PI),
    kind: "capsule" as const,
  },
  {
    x: 0,
    yBase: 3,
    rotation: (runtime: Box3DRuntime) => runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.25 * B3_PI),
    kind: "hull" as const,
  },
  {
    x: 3,
    yBase: 3,
    rotation: (runtime: Box3DRuntime) => runtime.makeQuatFromAxisAngle(B3_AXIS_X, 0.5 * B3_PI),
    kind: "mesh" as const,
  },
  {
    x: 5,
    yBase: 2,
    rotation: (runtime: Box3DRuntime) => runtime.makeQuatFromAxisAngle(B3_AXIS_X, -0.5 * B3_PI),
    kind: "heightfield" as const,
  },
] as const;

export interface OverlapWorldResources {
  mesh: MeshHandle;
  heightField: HeightFieldHandle;
}

export function buildOverlapWorldScene(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { handles: BodyHandle[]; resources: OverlapWorldResources } {
  const mesh = world.createTorusMesh(10, 12, 0.65, 0.35);
  const heightField = world.createWave(
    HF_ROW_COUNT,
    HF_COLUMN_COUNT,
    HF_SCALE,
    HF_ROW_FREQUENCY,
    HF_COLUMN_FREQUENCY,
    false,
  );

  const handles: BodyHandle[] = [];

  for (let index = 0; index < 3; index++) {
    const bodyType = index as BodyType;

    for (const shape of ROW_SHAPES) {
      const y = shape.yBase + 2 * index;
      const rotation = shape.rotation(runtime);
      const type = shape.kind === "heightfield" ? BodyType.Static : bodyType;
      const body = world.createBody({
        type,
        position: [shape.x, y, 0],
        rotation,
      });

      switch (shape.kind) {
        case "sphere":
          world.createSphereShape(body, [0, 0, 0], SPHERE_RADIUS, {});
          break;
        case "capsule":
          world.createCapsuleShape(body, CAPSULE.center1, CAPSULE.center2, CAPSULE.radius, {});
          break;
        case "hull":
          runtime.createHullShape(body, BOX_HALF, {});
          break;
        case "mesh":
          world.createMeshShape(body, mesh, { scale: MESH_SCALE });
          break;
        case "heightfield":
          world.createHeightFieldShape(body, heightField, {});
          break;
      }

      handles.push(body);
    }
  }

  return { handles, resources: { mesh, heightField } };
}

export function buildOverlapWorldDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return buildOverlapWorldScene(world, runtime).handles;
}

export function overlapWorldGroundSize(): Vec3 {
  return [20, 1, 20];
}

function overlapWorldBodyColor(kind: (typeof ROW_SHAPES)[number]["kind"], row: number): number {
  const palette = [0x60a5fa, 0x34d399, 0xa78bfa, 0xfbbf24, 0xf97316];
  const rowOffset = row * 5;
  const index = ROW_SHAPES.findIndex((shape) => shape.kind === kind);
  return palette[(rowOffset + index) % palette.length]!;
}

export const overlapWorldBodies: RenderBody[] = (() => {
  const bodies: RenderBody[] = [];
  for (let row = 0; row < 3; row++) {
    for (const shape of ROW_SHAPES) {
      const y = shape.yBase + 2 * row;
      const position: Vec3 = [shape.x, y, 0];
      const color = overlapWorldBodyColor(shape.kind, row);
      const type = shape.kind === "heightfield" ? BodyType.Static : (row as BodyType);

      switch (shape.kind) {
        case "sphere":
          bodies.push({ kind: "sphere", radius: SPHERE_RADIUS, position, rotation: [0, 0, 0, 1], type, color });
          break;
        case "capsule":
          bodies.push({
            kind: "capsule",
            axis: "x",
            radius: CAPSULE.radius,
            length: 1,
            position,
            rotation: [0, 0, 0, 1],
            type,
            color,
          });
          break;
        case "hull":
          bodies.push({ kind: "box", size: [1.2, 1.2, 1.2], position, rotation: [0, 0, 0, 1], type, color });
          break;
        case "mesh":
          bodies.push({ kind: "box", size: [0.01, 0.01, 0.01], position: [shape.x, -10, 0], type, color });
          break;
        case "heightfield":
          bodies.push({ kind: "box", size: [0.01, 0.01, 0.01], position: [shape.x, -10, 0], type: BodyType.Static, color });
          break;
      }
    }
  }
  return bodies;
})();

export const overlapWorldCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);

export const overlapWorldTorusVisual = {
  radialResolution: 10,
  tubularResolution: 12,
  radius: 0.65,
  thickness: 0.35,
  meshScale: MESH_SCALE,
} as const;

export const overlapWorldHeightFieldVisual = {
  rowCount: HF_ROW_COUNT,
  columnCount: HF_COLUMN_COUNT,
  scale: HF_SCALE,
  rowFrequency: HF_ROW_FREQUENCY,
  columnFrequency: HF_COLUMN_FREQUENCY,
} as const;

export const dumpSampleName = "Overlap World";
export const dumpSampleId = "collision/overlap-world";
export const dumpCppSampleName = "Overlap World";
export const dumpGroundSize = overlapWorldGroundSize;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, 0, 0], workerCount: 1 });
  const { handles, resources } = buildOverlapWorldScene(world, runtime);
  return {
    world,
    handles,
    dispose: () => {
      world.destroyMesh(resources.mesh);
      world.destroyHeightField(resources.heightField);
    },
  };
}
