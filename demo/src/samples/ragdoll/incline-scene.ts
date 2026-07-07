import { B3_AXIS_Z, B3_PI, BodyType, type Box3DRuntime, type HumanHandle, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { collectHumanBoneHandles, ragdollRenderBodies } from "./ragdoll-scene-shared";
import { f32 } from "../f32";

const HUMAN_ORIGIN: Vec3 = [-12, 6, 0];
const INCLINE_ANGLE = f32(-0.2 * B3_PI);
const INCLINE_RAMP_Q: [number, number, number, number] = [
  0,
  0,
  f32(Math.sin(INCLINE_ANGLE / 2)),
  f32(Math.cos(INCLINE_ANGLE / 2)),
];
const INCLINE_Q = (runtime: Box3DRuntime) => runtime.makeQuatFromAxisAngle(B3_AXIS_Z, INCLINE_ANGLE);

export function buildRagdollInclineDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  return buildRagdollInclineScene(world, runtime).handles;
}

export function buildRagdollInclineScene(world: PhysicsWorld, runtime: Box3DRuntime): { handles: number[]; human: HumanHandle } {
  const handles: number[] = [];
  const mesh = world.createGridMesh(4, 4, 2, 1, true);
  const inclineQ = INCLINE_Q(runtime);

  const ramp = world.createBody({ type: BodyType.Static, position: [-10, 2, 0], rotation: inclineQ });
  world.createMeshShape(ramp, mesh, { scale: [1, 1, 1] });
  handles.push(ramp);

  const flat = world.createBody({ type: BodyType.Static, position: [0, 0, 0] });
  world.createMeshShape(flat, mesh, { scale: [4, 4, 4] });
  handles.push(flat);

  const human = world.createHuman(HUMAN_ORIGIN, {
    frictionTorque: 10,
    hertz: 2,
    dampingRatio: 0.7,
    groupIndex: 1,
    colorize: false,
  });
  handles.push(...collectHumanBoneHandles(runtime, human));
  return { handles, human };
}

export function ragdollInclineGroundSize(): Vec3 { return [20, 1, 20]; }

export function createRagdollInclineBodies(): RenderBody[] {
  return [
    { kind: "box", size: [8, 0.2, 8], position: [-10, 2, 0], rotation: INCLINE_RAMP_Q, color: 0x94a3b8, type: BodyType.Static },
    { kind: "box", size: [32, 0.2, 32], position: [0, 0, 0], color: 0x64748b, type: BodyType.Static },
    ...ragdollRenderBodies(HUMAN_ORIGIN),
  ];
}

export const ragdollInclineCamera: RenderSpec["camera"] = { position: [-20, 30, 25], target: [0, 0, 0] };

export const dumpSampleName = "Incline";
export const dumpSampleId = "ragdoll/incline";
export const dumpCppSampleName = "Incline";
export const dumpGroundSize = ragdollInclineGroundSize;

interface InclineDumpState {
  human: HumanHandle;
  time: number;
  motorized: boolean;
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[]; state: InclineDumpState } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const { handles, human } = buildRagdollInclineScene(world, runtime);
  return { world, handles, state: { human, time: 0, motorized: true } };
}

export function dumpStep(_world: PhysicsWorld, runtime: Box3DRuntime, _handles: readonly number[], _frame: number, dt: number, state: InclineDumpState): void {
  if (state.time > 2 && state.motorized) {
    runtime.setHumanJointFrictionTorque(state.human, 0.5);
    runtime.setHumanJointSpringHertz(state.human, 0.5);
    state.motorized = false;
  }
  state.time = f32(state.time + dt);
}
