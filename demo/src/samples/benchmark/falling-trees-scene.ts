import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderPart, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";

export const FALLING_TREES_BODY_COUNT = 50;
export const FALLING_TREES_HULL_COUNT = 22;
export const FALLING_TREES_COLOR = 0x4ade80;

export type TreeScaleCm = 100 | 50 | 25;

export function treeScaleFromCm(cm: TreeScaleCm): number {
  if (cm === 100) return 1;
  if (cm === 50) return 2;
  return 4;
}

export function waveMeshParams(scale: number): {
  xCount: number;
  zCount: number;
  cellWidth: number;
  amplitude: number;
  rowFrequency: number;
  columnFrequency: number;
} {
  return {
    xCount: scale * 150,
    zCount: scale * 200,
    cellWidth: f32Div(1, scale),
    amplitude: f32(0.4),
    rowFrequency: f32(0.05),
    columnFrequency: f32(0.1),
  };
}

export interface FallingTreesState {
  mesh: MeshHandle | null;
}

export function buildTreeCylinderParts(): { height: number; radius: number; yOffset: number; localY: number }[] {
  const parts: { height: number; radius: number; yOffset: number; localY: number }[] = [];
  let y = f32(1);
  let r = f32(0.75);
  const l = f32(1.5);
  for (let i = 0; i < FALLING_TREES_HULL_COUNT; i++) {
    const height = f32Add(l, f32Mul(2, r));
    const yOffset = f32Sub(y, r);
    parts.push({
      height,
      radius: r,
      yOffset,
      localY: f32Add(yOffset, f32Mul(0.5, height)),
    });
    y = f32Add(y, height);
    r = f32Mul(f32(0.95), r);
  }
  return parts;
}

export function buildFallingTreesScene(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  scale = 1,
): { ground: BodyHandle; trees: BodyHandle[]; mesh: MeshHandle } {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const params = waveMeshParams(scale);
  const mesh = world.createWaveMesh(
    params.xCount,
    params.zCount,
    params.cellWidth,
    params.amplitude,
    params.rowFrequency,
    params.columnFrequency,
  );
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });

  const cylinderParts = buildTreeCylinderParts();
  const hulls = cylinderParts.map((part) =>
    runtime.createCylinder(part.height, part.radius, part.yOffset, 6),
  );

  const shapeOpts = {
    friction: 0.9,
    rollingResistance: 0.05,
    density: 1,
    updateBodyMass: false as const,
  };

  const handles: BodyHandle[] = [];
  let angularVelocity = f32(-0.5);
  let z = f32(-70);
  for (let bodyIndex = 0; bodyIndex < FALLING_TREES_BODY_COUNT; bodyIndex++) {
    const position: Vec3 = [0, 1, z];
    const body = world.createBody({
      type: BodyType.Dynamic,
      position,
      sleepThreshold: 0.2,
    });
    for (const hull of hulls) {
      runtime.createShapeFromHull(body, hull, shapeOpts);
    }

    const velocityScale = f32Add(f32(0.5), f32Div(f32Mul(f32(0.5), bodyIndex), FALLING_TREES_BODY_COUNT));
    runtime.applyBodyMassFromShapes(body);
    const center = world.getBodyWorldCenter(body);
    const omegaZ = f32Mul(velocityScale, angularVelocity);
    const omega: Vec3 = [0, 0, omegaZ];
    const dx = f32Sub(center[0], position[0]);
    const dy = f32Sub(center[1], position[1]);
    // cross(omega, center - position) with omega = (0,0,ωz)
    const v: Vec3 = [f32Mul(-omegaZ, dy), f32Mul(omegaZ, dx), 0];
    runtime.setBodyAngularVelocity(body, omega);
    world.setBodyLinearVelocity(body, v);

    handles.push(body);
    z = f32Add(z, f32(3));
    angularVelocity = f32(-angularVelocity);
  }

  for (const hull of hulls) runtime.destroyHull(hull);
  return { ground, trees: handles, mesh };
}

export function buildFallingTreesDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  return buildFallingTreesScene(world, runtime, 1).trees;
}

export function fallingTreesGroundSize(): Vec3 {
  return [75, 1, 100];
}

export function createFallingTreesBodies(): RenderBody[] {
  const parts = buildTreeCylinderParts();
  const renderParts = parts.map((part) => ({
    kind: "cylinder" as const,
    radius: part.radius,
    height: part.height,
    segments: 6,
    position: [0, part.localY, 0] as [number, number, number],
    color: FALLING_TREES_COLOR,
  }));
  const bodies: RenderBody[] = [];
  let z = -70;
  for (let i = 0; i < FALLING_TREES_BODY_COUNT; i++) {
    bodies.push({
      kind: "compound",
      position: [0, 1, z],
      parts: renderParts as [RenderPart, ...RenderPart[]],
    });
    z += 3;
  }
  return bodies;
}

export const fallingTreesCamera: RenderSpec["camera"] = cameraFromSetView(20, 0, 140, [0, 15, 0]);

export const dumpSampleName = "Falling Trees";
export const dumpSampleId = "benchmark/falling-trees";
export const dumpCppSampleName = "Falling Trees";

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  state: FallingTreesState;
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, trees, mesh } = buildFallingTreesScene(world, runtime, 1);
  return {
    world,
    handles: [ground, ...trees],
    state: { mesh },
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}
