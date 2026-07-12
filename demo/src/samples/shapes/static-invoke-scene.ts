import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export function buildStaticInvokeDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const body = world.createBody({ type: BodyType.Dynamic, position: [0.25, 1, 0] });
  runtime.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.2 });
  return [body];
}

export function createStaticInvokeStatic(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  invoke: boolean,
): BodyHandle {
  const body = world.createBody({ type: BodyType.Static, position: [0, 0.5, 0] });
  runtime.createSphereShape(body, [0, 0, 0], 0.5, { invokeContactCreation: invoke });
  return body;
}

export function createStaticInvoke(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyHandle[];
  state: { staticBody: BodyHandle | null; invoke: boolean };
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, [20, 1, 20]);
  return {
    world,
    handles: [ground, ...buildStaticInvokeDynamicBodies(world, runtime)],
    state: { staticBody: null, invoke: false },
  };
}

export function dumpPostStepStaticInvoke(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: BodyHandle[],
  frame: number,
  _dt: number,
  state: { staticBody: BodyHandle | null; invoke: boolean },
): void {
  // Upstream CreateStatic after Sample::Step when m_stepCount == 20.
  if (frame !== 20 || state.staticBody !== null) return;
  state.staticBody = createStaticInvokeStatic(world, runtime, state.invoke);
  handles.push(state.staticBody);
}

export function staticInvokeGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const staticInvokeBodies: RenderBody[] = [
  { kind: "sphere", radius: 0.5, position: [0.25, 1, 0], color: 0x60a5fa },
  { kind: "sphere", radius: 0.5, position: [0, 0.5, 0], color: 0x94a3b8 },
];

export const staticInvokeCamera: RenderSpec["camera"] = cameraFromSetView(0, 25, 10, [0, 1, 0]);

export const dumpSampleName = "Static Invoke";
export const dumpSampleId = "shapes/static-invoke";
export const dumpCppSampleName = "Static Invoke";
export const dumpCreate = createStaticInvoke;
export const dumpPostStep = dumpPostStepStaticInvoke;
