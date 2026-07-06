import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export function buildBridgeDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const hiddenGround = objectWorld.createBody();
  const handles: BodyHandle[] = [hiddenGround.handle];
  const a = 0.125;
  const xbase = -160 * a;
  let prev = hiddenGround;
  for (let i = 0; i < 150; i++) {
    const body = objectWorld.createBody({ type: BodyType.Dynamic, position: [xbase + a * (1 + 2 * i), 20, 0], linearDamping: 0.1, angularDamping: 0.1 });
    body.createHullShape([a, 0.125, 0.5], { density: 20 });
    objectWorld.createSphericalJoint(prev, body, { localFrameA: { position: prev.getLocalPoint([xbase + 2 * a * i, 20, -0.5]) }, localFrameB: { position: body.getLocalPoint([xbase + 2 * a * i, 20, -0.5]) }, enableSpring: true, hertz: 2, dampingRatio: 1 });
    objectWorld.createSphericalJoint(prev, body, { localFrameA: { position: prev.getLocalPoint([xbase + 2 * a * i, 20, 0.5]) }, localFrameB: { position: body.getLocalPoint([xbase + 2 * a * i, 20, 0.5]) }, enableSpring: true, hertz: 2, dampingRatio: 1 });
    handles.push(body.handle);
    prev = body;
  }
  objectWorld.createSphericalJoint(prev, hiddenGround, { localFrameA: { position: prev.getLocalPoint([xbase + 2 * a * 150, 20, -0.5]) }, localFrameB: { position: hiddenGround.getLocalPoint([xbase + 2 * a * 150, 20, -0.5]) }, enableSpring: true, hertz: 2, dampingRatio: 1 });
  objectWorld.createSphericalJoint(prev, hiddenGround, { localFrameA: { position: prev.getLocalPoint([xbase + 2 * a * 150, 20, 0.5]) }, localFrameB: { position: hiddenGround.getLocalPoint([xbase + 2 * a * 150, 20, 0.5]) }, enableSpring: true, hertz: 2, dampingRatio: 1 });
  return handles;
}

export const bridgeGroundSize = (): Vec3 => [60, 1, 60];
export const bridgeCamera = { position: [0, 20, 35] as [number, number, number], target: [0, 10, 0] as [number, number, number] };
export const dumpSampleName = "Bridge";
export const dumpSampleId = "joints/bridge";
export const dumpCppSampleName = "Bridge";
export const dumpGroundSize = bridgeGroundSize;
export const dumpBuildDynamicBodies = buildBridgeDynamicBodies;
