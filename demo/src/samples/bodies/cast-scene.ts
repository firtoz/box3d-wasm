import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const BODY_POSITION: Vec3 = [5, 5, 0];
const BODY_ANGULAR_VELOCITY: Vec3 = [0.1, -0.1, 0.1];

const CYLINDER_HEIGHT = 2.0;
const CYLINDER_RADIUS = 0.5;
const CYLINDER_SIDES = 16;

export function buildBodyCastDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const body = world.createBody({
    type: BodyType.Kinematic,
    position: BODY_POSITION,
    angularVelocity: BODY_ANGULAR_VELOCITY,
  });

  const cylinder = runtime.createCylinder(CYLINDER_HEIGHT, CYLINDER_RADIUS, 0, CYLINDER_SIDES);
  runtime.createShapeFromHull(body, cylinder);
  runtime.destroyHull(cylinder);

  return [body];
}

export function bodyCastGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const bodyCastBodies: RenderBody[] = [
  {
    kind: "cylinder",
    radius: CYLINDER_RADIUS,
    height: CYLINDER_HEIGHT,
    segments: CYLINDER_SIDES,
    position: BODY_POSITION,
    type: BodyType.Kinematic,
    color: 0x60a5fa,
  },
];

export const bodyCastCamera: RenderSpec["camera"] = cameraFromSetView(120, 30, 20, [0, 1.5, 0]);

export const dumpSampleName = "Cast";
export const dumpSampleId = "bodies/cast";
export const dumpCppSampleName = "Cast";
export const dumpGroundSize = bodyCastGroundSize;

export function createBodyCast(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  return { world, handles: buildBodyCastDynamicBodies(world, runtime) };
}

export const dumpCreate = createBodyCast;
