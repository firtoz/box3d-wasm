import type { Box3DRuntime, HumanHandle, Vec3 } from "box3d-wasm";
import type { RenderBody } from "../generic-host";
import { RAGDOLL_RENDER_BONES } from "../../ragdoll-render";

export function collectHumanBoneHandles(runtime: Box3DRuntime, human: HumanHandle): number[] {
  const handles: number[] = [];
  const boneCount = runtime.getHumanBoneCount();
  for (let i = 0; i < boneCount; i++) {
    const boneBody = runtime.getHumanBoneBody(human, i);
    if (boneBody !== 0) handles.push(boneBody);
  }
  return handles;
}

/** Match thrown-ragdoll rendering: capsule geometry baked from bone local endpoints. */
export function ragdollRenderBodies(origin: Vec3): RenderBody[] {
  const [ox, oy, oz] = origin;
  return RAGDOLL_RENDER_BONES.map((bone) => ({
    kind: "ragdoll-capsule" as const,
    a: bone.a,
    b: bone.b,
    radius: bone.radius,
    color: bone.color,
    position: [ox + bone.position[0], oy + bone.position[1], oz + bone.position[2]],
    rotation: bone.rotation,
  }));
}
