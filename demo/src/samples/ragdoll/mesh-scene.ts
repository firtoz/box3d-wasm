import {
  BodyType,
  type BodyHandle,
  type Box3DRuntime,
  type MeshHandle,
  type PhysicsWorld,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import {
  collectHumanBoneAndAnchorHandles,
  ragdollRenderBodies,
} from "./ragdoll-scene-shared";

const HUMAN_ORIGIN: Vec3 = [0, 1, 0];

const WALL_SPECS: ReadonlyArray<{ position: Vec3; halfExtents: Vec3 }> = [
  { position: [0, 5, -20], halfExtents: [20, 5, 0.1] },
  { position: [0, 5, 20], halfExtents: [20, 5, 0.1] },
  { position: [-20, 5, 0], halfExtents: [0.1, 5, 20] },
  { position: [20, 5, 0], halfExtents: [0.1, 5, 20] },
];

export function buildRagdollMeshGround(world: PhysicsWorld, runtime: Box3DRuntime): { ground: BodyHandle; mesh: MeshHandle } {
  const ground = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  const mesh = world.createGridMesh(20, 20, 2, 2, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });

  for (const wall of WALL_SPECS) {
    runtime.createTransformedHullShape(ground, wall.halfExtents, { position: wall.position });
  }

  return { ground, mesh };
}

export function buildRagdollMeshDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const human = world.createHuman(HUMAN_ORIGIN, {
    frictionTorque: 5,
    hertz: 2,
    dampingRatio: 0.7,
    groupIndex: 1,
    colorize: false,
  });
  runtime.createHumanParallelAnchors(human);
  return collectHumanBoneAndAnchorHandles(runtime, human);
}

export function ragdollMeshGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function createRagdollMeshBodies(): RenderBody[] {
  return ragdollRenderBodies(HUMAN_ORIGIN);
}

export const ragdollMeshCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 6, [0, 0, 0]);

export const dumpSampleName = "Mesh";
export const dumpSampleId = "ragdoll/mesh";
export const dumpCppSampleName = "Ragdoll/Mesh";
export const dumpGroundSize = ragdollMeshGroundSize;

export function createRagdollMesh(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: number[];
  dispose: () => void;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { ground, mesh } = buildRagdollMeshGround(world, runtime);
  return {
    world,
    handles: [ground, ...buildRagdollMeshDynamicBodies(world, runtime)],
    dispose: () => {
      world.destroyMesh(mesh);
    },
  };
}

export const dumpCreate = createRagdollMesh;
