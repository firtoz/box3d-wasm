import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";
import { f32 } from "../f32";

const LENGTH = f32(1);
const Y_OFFSET = f32(20);
const RADIUS = f32(0.25);
const DENSITY = f32(20);

export function buildDistanceJointDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const anchor = objectWorld.createBody();
  const body = objectWorld.createBody({
    type: BodyType.Dynamic,
    position: [LENGTH, Y_OFFSET, 0],
    angularDamping: 1,
  });
  body.createSphereShape([0, 0, 0], RADIUS, { density: DENSITY });

  const pivotA = anchor.getLocalPointXYZ(0, Y_OFFSET, 0);
  const pivotB = body.getLocalPointXYZ(LENGTH, Y_OFFSET, 0);
  objectWorld.createDistanceJoint(anchor, body, {
    localFrameA: { position: pivotA },
    localFrameB: { position: pivotB },
    length: LENGTH,
  });

  return [anchor.handle, body.handle];
}

export function distanceJointGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const distanceJointBodies: RenderBody[] = [
  { kind: "sphere", radius: RADIUS, position: [LENGTH, Y_OFFSET, 0], color: 0x38bdf8 },
];

export const distanceJointCamera: RenderSpec["camera"] = cameraFromSetView(0, 0, 40, [0, 10, 0]);

export const dumpSampleName = "Distance Joint";
export const dumpSampleId = "joints/distance-joint";
export const dumpCppSampleName = "Distance Joint";
export const dumpGroundSize = distanceJointGroundSize;
export const dumpBuildDynamicBodies = buildDistanceJointDynamicBodies;
