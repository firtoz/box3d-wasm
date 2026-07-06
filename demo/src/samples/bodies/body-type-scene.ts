import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";
import type { RenderBody, RenderSpec } from "../generic-host";

const PI = Math.PI;

// Ground body is handle 1 (first body created in the world).
const groundHandle = 1 as BodyHandle;

export function buildBodyTypeDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const handles: BodyHandle[] = [];
  const objectRuntime = ObjectRuntime.fromRuntime(runtime);
  const objectWorld = objectRuntime.wrapWorld(world);
  const ground = objectWorld.body(groundHandle);

  const a = objectWorld.createBody({ type: BodyType.Dynamic, position: [-2, 3, 0] });
  a.createHullShape([0.5, 2, 0.5], { density: 1 });
  handles.push(a.handle);

  const b = objectWorld.createBody({ type: BodyType.Dynamic, position: [3, 3, 0] });
  b.createHullShape([0.5, 2, 0.5], { density: 1 });
  handles.push(b.handle);

  const p = objectWorld.createBody({ type: BodyType.Dynamic, position: [-4, 5, 0] });
  p.createTransformedHullShape([0.5, 4, 0.5], {
    position: [4, 0, 0],
    rotation: [0, 0, Math.sin(PI / 4), Math.cos(PI / 4)],
  }, undefined, { density: 2 });
  handles.push(p.handle);

  objectWorld.createRevoluteJoint(a, p, {
    localFrameA: { position: a.getLocalPoint([-2, 5, 0]) },
    localFrameB: { position: p.getLocalPoint([-2, 5, 0]) },
    enableMotor: true,
    maxMotorTorque: 50,
  });

  objectWorld.createRevoluteJoint(b, p, {
    localFrameA: { position: b.getLocalPoint([3, 5, 0]) },
    localFrameB: { position: p.getLocalPoint([3, 5, 0]) },
    enableMotor: true,
    maxMotorTorque: 50,
  });

  objectWorld.createPrismaticJoint(ground, p, {
    localFrameA: { position: ground.getLocalPoint([0, 5, 0]) },
    localFrameB: { position: p.getLocalPoint([0, 5, 0]) },
    enableMotor: true,
    maxMotorForce: 1000,
    motorSpeed: 0,
    enableLimit: true,
    lowerTranslation: -10,
    upperTranslation: 10,
  });

  const c = objectWorld.createBody({ type: BodyType.Dynamic, position: [-3, 8, 0] });
  c.createHullShape([0.75, 0.75, 0.75], { density: 2 });
  handles.push(c.handle);

  const d = objectWorld.createBody({ type: BodyType.Dynamic, position: [2, 8, 0] });
  d.createHullShape([0.75, 0.75, 0.75], { density: 2 });
  handles.push(d.handle);

  const e = objectWorld.createBody({ type: BodyType.Dynamic, position: [8, 0.2, 0] });
  e.createCapsuleShape([0, 0, 0], [1, 0, 0], 0.25, { density: 2 });
  handles.push(e.handle);

  const f = objectWorld.createBody({ type: BodyType.Dynamic, position: [-8, 12, 0], gravityScale: 0 });
  f.createSphereShape([0, 0.5, 0], 0.25, { density: 2 });
  handles.push(f.handle);

  return handles;
}

export function bodyTypeGroundSize(): Vec3 {
  return [20, 1, 20];
}

// attachment, secondAttachment, platform, crate1, secondPayload, touchingBody, floatingBody
const idx: [number, number, number, number, number, number, number] = [0, 1, 2, 3, 4, 5, 6];

export const bodyTypeHandleIndex = {
  attachmentId: idx[0],
  secondAttachmentId: idx[1],
  platformId: idx[2],
  crate1: idx[3],
  secondPayloadId: idx[4],
  touchingBodyId: idx[5],
  floatingBodyId: idx[6],
};

export function stepBodyType(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  platformId: BodyHandle,
  bodyType: number,
  platformVx: number,
): number {
  if (bodyType === BodyType.Kinematic) {
    const pos = world.getBodyTransform(platformId).position;
    if ((pos[0] < -14 && platformVx < 0) || (pos[0] > 6 && platformVx > 0)) {
      platformVx = -platformVx;
    }
    runtime.setBodyLinearVelocity(platformId, [platformVx, 0, 0]);
  }
  return platformVx;
}

// The platform hull is a box of half-extents [0.5, 4, 0.5] transformed by
// translate(4,0,0) + Z-90°. After transformation, the hull spans
// local X: [0, 8], Y: [-0.5, 0.5], Z: [-0.5, 0.5], center at [4, 0, 0] in body space.
export const bodyTypeBodies: RenderBody[] = [
  { kind: "box", size: [1, 4, 1], position: [-2, 3, 0], color: 0x3b82f6 },
  { kind: "box", size: [1, 4, 1], position: [3, 3, 0], color: 0xf97316 },
  {
    kind: "compound",
    position: [-4, 5, 0],
    parts: [
      { kind: "box", size: [8, 1, 1], position: [4, 0, 0], color: 0x22c55e },
    ],
  },
  { kind: "box", size: [1.5, 1.5, 1.5], position: [-3, 8, 0], color: 0xef4444 },
  { kind: "box", size: [1.5, 1.5, 1.5], position: [2, 8, 0], color: 0xa855f7 },
  { kind: "capsule", radius: 0.25, length: 1, axis: "x", position: [8, 0.2, 0], localPosition: [0.5, 0, 0], color: 0xf59e0b },
  { kind: "sphere", radius: 0.25, position: [-8, 12, 0], color: 0x888888 },
];

export const bodyTypeCamera: RenderSpec["camera"] = { position: [0, 30, 30], target: [0, 1.5, 0] };

export const dumpSampleName = "Body Type";
export const dumpSampleId = "bodies/body-type";
export const dumpCppSampleName = "Body Type";
export const dumpGroundSize = bodyTypeGroundSize;
export const dumpBuildDynamicBodies = buildBodyTypeDynamicBodies;
