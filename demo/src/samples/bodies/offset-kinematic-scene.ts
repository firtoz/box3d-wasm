import {BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3, type BodyId} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const OFFSET: Vec3 = [0.5, 0.5, 1.0];
const HALF_WIDTHS: Vec3 = [0.5, 0.5, 1.0];

export function buildOffsetKinematicDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const body = world.createBody({
    type: BodyType.Kinematic,
    position: [0, 3, 0],
  });
  const hull = runtime.makeTransformedBoxHull(HALF_WIDTHS, { position: OFFSET });
  runtime.createShapeFromHull(body, hull);
  const mass = runtime.getBodyMassData(body).mass;
  const inertia = runtime.getBodyLocalRotationalInertia(body);
  runtime.setBodyMassData(body, mass, OFFSET, inertia);
  runtime.setBodyAngularVelocity(body, [1, -1, 2]);
  return [body];
}

export function offsetKinematicGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const offsetKinematicBodies: RenderBody[] = [
  {
    kind: "box",
    size: [1, 1, 2],
    position: [0, 3, 0],
    localPosition: OFFSET,
    color: 0x3b82f6,
  },
];

export const offsetKinematicCamera: RenderSpec["camera"] = { position: [0, 30, 10], target: [0, 1.5, 0] };

export const dumpSampleName = "Offset Kinematic";
export const dumpSampleId = "bodies/offset-kinematic";
export const dumpCppSampleName = "Offset Kinematic";
export const dumpGroundSize = offsetKinematicGroundSize;
export const dumpBuildDynamicBodies = buildOffsetKinematicDynamicBodies;
