import { B3_AXIS_Z, BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Mat3, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

function steinerAdjustedInertia(inertia: Mat3, mass: number, offset: Vec3): Mat3 {
  const r2 = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
  const out = [...inertia] as unknown as Mat3;
  for (let i = 0; i < 3; i++) {
    for (let j = 0; j < 3; j++) {
      out[i * 3 + j] += mass * (r2 * (i === j ? 1 : 0) - offset[i] * offset[j]);
    }
  }
  return out;
}

export function buildWeebleDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const body = world.createBody({ type: BodyType.Dynamic, position: [0, 3, 0] });

  runtime.createCapsuleShape(body, [0, -1, 0], [0, 1, 0], 1, {
    rollingResistance: 0.1,
  });

  const mass = runtime.getBodyMassData(body).mass;
  const inertia = runtime.getBodyLocalRotationalInertia(body);
  const offset: Vec3 = [0, -1.5, 0];
  const adjusted = steinerAdjustedInertia(inertia, mass, offset);
  runtime.setBodyMassData(body, mass, offset, adjusted);

  return [body];
}

export function teleportWeeble(runtime: Box3DRuntime, body: BodyHandle): void {
  runtime.setBodyTransform(body, [0, 5, 0], runtime.makeQuatFromAxisAngle(B3_AXIS_Z, 0.95 * Math.PI));
  runtime.setBodyAwake(body, true);
}

export const WEEBLE_EXPLOSION_DEFAULT = 20000;
export const WEEBLE_EXPLOSION_MIN = -100000;
export const WEEBLE_EXPLOSION_MAX = 100000;

export function explodeWeeble(world: PhysicsWorld, magnitude = WEEBLE_EXPLOSION_DEFAULT): void {
  world.explode([0, -0.1, 0], 8, 0.1, magnitude, 0xFFFFFFFFn as unknown as number);
}

export function weebleGroundSize(): Vec3 {
  return [30, 1, 30];
}

export const weebleBodies: RenderBody[] = [
  { kind: "capsule", radius: 1, length: 2, axis: "y", position: [0, 3, 0], color: 0x3b82f6 },
];

export const weebleCamera: RenderSpec["camera"] = { position: [45, 25, 25], target: [0, 0, 0] };

export const dumpSampleName = "Weeble";
export const dumpSampleId = "bodies/weeble";
export const dumpCppSampleName = "Weeble";
export const dumpGroundSize = weebleGroundSize;
export const dumpBuildDynamicBodies = buildWeebleDynamicBodies;
export const dumpInteractionSchedule = [
  { frame: 250, action: "teleport" },
] as const;

export function dumpRunInteraction(_world: PhysicsWorld, runtime: Box3DRuntime, handles: readonly BodyHandle[], interaction: { action: string }): void {
  if (interaction.action !== "teleport") throw new Error(`Unsupported weeble dump action: ${interaction.action}`);
  teleportWeeble(runtime, handles[1]!);
}
