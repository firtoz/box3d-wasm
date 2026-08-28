import { type BodyId, type Box3DRuntime, type HumanHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { collectHumanBoneHandles, ragdollRenderBodies } from "../ragdoll/ragdoll-scene-shared";

const ORIGIN: Vec3 = [0, 2, 0];

export function buildExtraParallelAnchorsDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  human: HumanHandle;
} {
  const human = world.createHuman(ORIGIN, {
    frictionTorque: 1,
    hertz: 1,
    dampingRatio: 0.7,
    groupIndex: 1,
    colorize: false,
  });
  runtime.createHumanParallelAnchors(human);
  return { handles: collectHumanBoneHandles(runtime, human), human };
}

export function extraParallelAnchorsGroundSize(): Vec3 {
  return [20, 1, 20];
}

export function extraParallelAnchorsBodies(): RenderBody[] {
  return ragdollRenderBodies(ORIGIN);
}

export const extraParallelAnchorsCamera: RenderSpec["camera"] = cameraFromSetView(20, 18, 14, [0, 1, 0]);

export const EXTRA_PARALLEL_ANCHORS_HUD_INTS = 2;
