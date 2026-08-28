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

/** Upstream `BodyCast::Step` never calls `Sample::Step`, so the world does not advance. */
export const dumpOwnsStep = true;

export const dumpCreate = createBodyCast;

export const BODY_CAST_HUD_FLOATS = 17;
export const bodyCastRayOrigin: Vec3 = [-9.75, 3, -4];
export const bodyCastRayTranslation: Vec3 = [0, 0, 8];
export const bodyCastSphereOrigin: Vec3 = [-14.5, 2.5, 0.5];
export const bodyCastSphereTranslation: Vec3 = [8, 0, 0];
export const bodyCastSphereRadius = 0.2;
export const bodyCastOverlapOrigin: Vec3 = [-10, 1, 0.5];
export const bodyCastOverlapCapsule = { a: [-0.5, 1, 0] as Vec3, b: [0.5, 0, 0] as Vec3, radius: 0.5 };

export function bodyCastQueryTransform(runtime: Box3DRuntime): { position: Vec3; rotation: [number, number, number, number] } {
  const { direction } = runtime.getLengthAndNormalize([1, -2, 3]);
  return {
    position: [-10, 2, 0],
    rotation: runtime.makeQuatFromAxisAngle(direction, 0.75 * Math.PI),
  };
}

export function fillBodyCastHud(world: PhysicsWorld, bodyHandle: BodyHandle, xf: { position: Vec3; rotation: [number, number, number, number] }, out: Float32Array): void {
  const ray = world.bodyCastRay(bodyHandle, bodyCastRayOrigin, bodyCastRayTranslation, { bodyTransform: xf });
  out[0] = ray.hit ? 1 : 0;
  out[1] = ray.fraction;
  out[2] = ray.point[0];
  out[3] = ray.point[1];
  out[4] = ray.point[2];
  out[5] = ray.normal[0];
  out[6] = ray.normal[1];
  out[7] = ray.normal[2];
  const sphere = world.bodyCastShape(bodyHandle, bodyCastSphereOrigin, bodyCastSphereTranslation, {
    radius: bodyCastSphereRadius,
    canEncroach: true,
    bodyTransform: xf,
  });
  out[8] = sphere.hit ? 1 : 0;
  out[9] = sphere.fraction;
  out[10] = sphere.point[0];
  out[11] = sphere.point[1];
  out[12] = sphere.point[2];
  out[13] = sphere.normal[0];
  out[14] = sphere.normal[1];
  out[15] = sphere.normal[2];
  out[16] = world.bodyOverlapShape(bodyHandle, bodyCastOverlapOrigin, bodyCastOverlapCapsule, { bodyTransform: xf }) ? 1 : 0;
}
