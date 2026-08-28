import { B3_PI, BodyType, type BodyId, type Box3DRuntime, type HumanHandle, type MeshHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { collectHumanBoneHandles, ragdollRenderBodies } from "../ragdoll/ragdoll-scene-shared";
import { cameraFromSetView } from "../shared";

export function buildPoseScene(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  human: HumanHandle;
  mesh: MeshHandle;
} {
  const mesh = world.createGridMesh(4, 2, 2, 1, true);
  const ground = world.createBody();
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  runtime.createOffsetHullShape(ground, [0.25, 0.5, 3], [-3, 0.5, 0], {});
  const human = world.createHuman([0, 0.1, 0], { frictionTorque: 5, hertz: 1, dampingRatio: 0.7, groupIndex: 1, colorize: false });
  const cylQ = runtime.makeQuatFromAxisAngle([0, 0, 1], 0.5 * B3_PI);
  const cylinderBody = world.createBody({ type: BodyType.Dynamic, position: [3, 0.5, 0], rotation: cylQ });
  const hull = runtime.createCylinder(0.5, 0.5, 0, 16);
  runtime.createShapeFromHull(cylinderBody, hull, {});
  runtime.destroyHull(hull);
  return { handles: [ground, ...collectHumanBoneHandles(runtime, human), cylinderBody], human, mesh };
}

export function poseGroundSize(): Vec3 {
  return [10, 1, 10];
}

export function createPoseBodies(): RenderBody[] {
  return [
    { kind: "box", size: [16, 0.1, 8], position: [0, 0, 0], type: BodyType.Static, color: 0x475569 },
    ...ragdollRenderBodies([0, 0.1, 0]),
    { kind: "cylinder", radius: 0.5, height: 0.5, position: [3, 0.5, 0], color: 0xf59e0b },
  ];
}

export const poseCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 6, [0, 0, 0]);
