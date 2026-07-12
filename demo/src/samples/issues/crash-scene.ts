import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const BOX_HALF = 0.5;
const BOX_SIZE: [number, number, number] = [2 * BOX_HALF, 2 * BOX_HALF, 2 * BOX_HALF];

export function buildCrashGround(world: PhysicsWorld): BodyHandle {
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  const mesh = world.createGridMesh(20, 20, 2, 0, true);
  world.createMeshShape(ground, mesh, { scale: [1, 1, 1] });
  return ground;
}

export function buildCrashDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const body1 = world.createBody({ type: BodyType.Dynamic, position: [2, 4, 0] });
  runtime.createHullShape(body1, [BOX_HALF, BOX_HALF, BOX_HALF]);
  const body2 = world.createBody({ type: BodyType.Dynamic, position: [-2, 4, 0] });
  runtime.createHullShape(body2, [BOX_HALF, BOX_HALF, BOX_HALF]);
  return [body1, body2];
}

export function createCrash(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = buildCrashGround(world);
  return { world, handles: [ground, ...buildCrashDynamicBodies(world, runtime)] };
}

export function crashGroundSize(): Vec3 {
  return [20, 0.5, 20];
}

export const crashBodies: RenderBody[] = [
  { kind: "box", size: BOX_SIZE, position: [2, 4, 0], color: 0x60a5fa },
  { kind: "box", size: BOX_SIZE, position: [-2, 4, 0], color: 0xf97316 },
];

export const crashCamera: RenderSpec["camera"] = cameraFromSetView(45, 30, 15, [0, 2, 0]);

export const dumpSampleName = "Crash";
export const dumpSampleId = "issues/crash";
export const dumpCppSampleName = "Crash";
export const dumpCreate = createCrash;

/** Weld the two dynamic boxes (same as live Add Joint). */
export function dumpRunInteraction(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  handles: readonly BodyHandle[],
  interaction: { action: string },
): void {
  if (interaction.action !== "add-joint") throw new Error(`Unsupported crash dump action: ${interaction.action}`);
  const body1 = handles[1];
  const body2 = handles[2];
  if (body1 === undefined || body2 === undefined) throw new Error("Crash dump expects [ground, body1, body2]");
  world.createWeldJoint(body1, body2);
}

/** Dump-only aliases covering Add Joint before vs after sleep. */
export const dumpVariants = [
  {
    id: "issues/crash-joint-awake",
    name: "Issues / Crash (joint while awake)",
    cppName: "Crash Joint Awake",
    interactionSchedule: [{ frame: 10, action: "add-joint" }],
  },
  {
    id: "issues/crash-joint-asleep",
    name: "Issues / Crash (joint while asleep)",
    cppName: "Crash Joint Asleep",
    interactionSchedule: [{ frame: 200, action: "add-joint" }],
  },
] as const;
