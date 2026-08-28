import { BodyType, type BodyId, type Box3DRuntime, type HumanHandle, type MeshHandle, type PhysicsWorld, type ShapeId, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { collectHumanBoneHandles, ragdollRenderBodies } from "../ragdoll/ragdoll-scene-shared";
import { f32Add, f32Mul } from "../f32";
import { cameraFromSetView, getWasmBaseUrl } from "../shared";
import { parseObjText } from "../meshes/parse-obj";

const HUMAN_COUNT = 20;

function humanPosition(index: number): Vec3 {
  return [f32Add(-14, f32Mul(1.5, index)), 8, 0];
}

const BUILDING_MATERIALS = [
  { friction: 0.6 },
  { friction: 0, restitution: 0.95, userMaterialId: 1 },
  { friction: 0.2, restitution: 0.2, userMaterialId: 2 },
];

export function buildReflectionCore(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  humans: HumanHandle[];
  gridMesh: MeshHandle;
  meshShape: ShapeId | null;
  meshBody: BodyId;
} {
  const handles: BodyId[] = [];
  const humans: HumanHandle[] = [];
  const grid = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const gridMesh = world.createGridMesh(20, 20, 2, 2, true);
  world.createMeshShape(grid, gridMesh, { scale: [1, 1, 1] });
  handles.push(grid);

  const left = world.createBody({ position: [-10, 0, 0] });
  handles.push(left);
  const meshBody = world.createBody({ position: [10, 0, 0] });
  handles.push(meshBody);

  const sphere = world.createBody({ type: BodyType.Dynamic, position: [6, 15, 0] });
  runtime.createSphereShape(sphere, [0, 0, 0], 0.5, { rollingResistance: 0.2, userMaterialId: 42 });
  handles.push(sphere);

  const capsule = world.createBody({ type: BodyType.Dynamic, position: [9, 15, 0] });
  runtime.createCapsuleShape(capsule, [-0.5, 0.5, 0], [0.5, 0, 0], 0.25, { rollingResistance: 0.2, userMaterialId: 11 });
  handles.push(capsule);

  const box = world.createBody({ type: BodyType.Dynamic, position: [12, 15, 0] });
  runtime.createHullShape(box, [0.25, 0.5, 0.75], { userMaterialId: 555 });
  handles.push(box);

  for (let i = 0; i < HUMAN_COUNT; i++) {
    const human = world.createHuman(humanPosition(i), {
      frictionTorque: 5,
      hertz: 1,
      dampingRatio: 0.7,
      groupIndex: i,
      colorize: false,
    });
    humans.push(human);
    handles.push(...collectHumanBoneHandles(runtime, human));
  }

  return { handles, humans, gridMesh, meshShape: null, meshBody };
}

export function attachBuildingMeshes(
  world: PhysicsWorld,
  vertices: number[],
  indices: number[],
  leftBody: BodyId,
  meshBody: BodyId,
  scale: Vec3,
): { mesh: MeshHandle; meshShape: ShapeId } {
  const mesh = world.createMesh(vertices, indices, { useMedianSplit: false, identifyEdges: true, weldVertices: true });
  world.createMeshShape(leftBody, mesh, { scale: [1, 1, 1], surfaceMaterials: BUILDING_MATERIALS });
  const shaped = world.createMeshShape(meshBody, mesh, { scale, surfaceMaterials: BUILDING_MATERIALS });
  return { mesh, meshShape: shaped.shapeHandle };
}

export async function loadBuildingObj(): Promise<{ vertices: number[]; indices: number[] }> {
  const response = await fetch(`${getWasmBaseUrl()}meshes/building.obj`);
  const text = await response.text();
  return parseObjText(text);
}

export function reflectionGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createReflectionBodies(): RenderBody[] {
  const bodies: RenderBody[] = [
    { kind: "box", size: [40, 0.2, 40], position: [0, 0, 0], type: BodyType.Static, color: 0x334155 },
    { kind: "box", size: [8, 8, 8], position: [-10, 4, 0], type: BodyType.Static, color: 0x64748b },
    { kind: "box", size: [8, 8, 8], position: [10, 4, 0], type: BodyType.Static, color: 0x94a3b8 },
    { kind: "sphere", radius: 0.5, position: [6, 15, 0], color: 0x38bdf8 },
    { kind: "capsule", axis: "x", radius: 0.25, length: 1, position: [9, 15, 0], color: 0xa78bfa },
    { kind: "box", size: [0.5, 1, 1.5], position: [12, 15, 0], color: 0xf59e0b },
  ];
  for (let i = 0; i < HUMAN_COUNT; i++) bodies.push(...ragdollRenderBodies(humanPosition(i)));
  return bodies;
}

export const reflectionCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 40, [0, 0, 0]);

export const dumpSampleName = "Reflection";
export const dumpSampleId = "mesh/reflection";
export const dumpCppSampleName = "Reflection";
export const dumpGroundSize = reflectionGroundSize;

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const built = buildReflectionCore(world, runtime);
  return {
    world,
    handles: built.handles,
    dispose: () => {
      world.destroyMesh(built.gridMesh);
    },
  };
}
