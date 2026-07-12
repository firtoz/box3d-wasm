import { BodyType, type BodyHandle, type BodyTransform, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";

export type TopDownFrictionVisible =
  | ({ kind: "capsule"; radius: number; length: number } & BodyTransform)
  | ({ kind: "sphere"; radius: number } & BodyTransform)
  | ({ kind: "box"; size: Vec3 } & BodyTransform);

export function createTopDownFriction(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0] });

  const ground = world.createBody();
  world.createTransformedHullShape(ground, [10, 0.5, 4], { position: [0, 0, 0] });
  world.createTransformedHullShape(ground, [0.5, 10, 4], { position: [-10, 10, 0] });
  world.createTransformedHullShape(ground, [0.5, 10, 4], { position: [10, 10, 0] });
  world.createTransformedHullShape(ground, [10, 0.5, 4], { position: [0, 20, 0] });

  const handles: BodyHandle[] = [ground];
  const capsuleRadius = 0.25;
  const sphereRadius = 0.35;
  const cubeHalf: Vec3 = [0.35, 0.35, 0.35];

  let x = -5;
  let y = 15;
  for (let i = 0; i < 10; i++) {
    for (let j = 0; j < 10; j++) {
      const body = world.createBody({ type: BodyType.Dynamic, position: [x, y, 0], gravityScale: 0 });
      const remainder = (10 * i + j) % 4;
      if (remainder === 0) {
        world.createCapsuleShape(body, [-0.25, 0, 0], [0.25, 0, 0], capsuleRadius, { restitution: 0.8 });
      } else if (remainder === 1) {
        world.createSphereShape(body, [0, 0, 0], sphereRadius, { restitution: 0.8 });
      } else {
        world.createHullShape(body, cubeHalf, { restitution: 0.8 });
      }
      world.createMotorJoint(ground, body, { collideConnected: true, maxVelocityForce: 1000, maxVelocityTorque: 1000 });
      handles.push(body);
      x += 1;
    }
    x = -5;
    y -= 1;
  }

  return { world, handles };
}

export function createTopDownFrictionVisible(): TopDownFrictionVisible[] {
  const visible: TopDownFrictionVisible[] = [];
  let x = -5;
  let y = 15;
  for (let i = 0; i < 10; i++) {
    for (let j = 0; j < 10; j++) {
      const remainder = (10 * i + j) % 4;
      if (remainder === 0) visible.push({ kind: "capsule", radius: 0.25, length: 0.5, position: [x, y, 0], rotation: [0, 0, 0, 1] });
      else if (remainder === 1) visible.push({ kind: "sphere", radius: 0.35, position: [x, y, 0], rotation: [0, 0, 0, 1] });
      else visible.push({ kind: "box", size: [0.7, 0.7, 0.7], position: [x, y, 0], rotation: [0, 0, 0, 1] });
      x += 1;
    }
    x = -5;
    y -= 1;
  }
  return visible;
}

export const topDownFrictionCamera = { position: [0, 10, 26] as [number, number, number], target: [0, 10, 0] as [number, number, number] };
export const topDownFrictionGroundParts = [
  { size: [20, 1, 8] as Vec3, position: [0, 0, 0] as Vec3 },
  { size: [1, 20, 8] as Vec3, position: [-10, 10, 0] as Vec3 },
  { size: [1, 20, 8] as Vec3, position: [10, 10, 0] as Vec3 },
  { size: [20, 1, 8] as Vec3, position: [0, 20, 0] as Vec3 },
] as const;

export const dumpSampleName = "Top Down Friction";
export const dumpSampleId = "joints/top-down-friction";
export const dumpCppSampleName = "Top Down Friction";
export const dumpInteractionSchedule = [
  { frame: 1, action: "explode", args: [0, 10, 0, 10, 5, 10000] },
] as const;

export function dumpRunInteraction(world: PhysicsWorld, _runtime: Box3DRuntime, _handles: readonly BodyHandle[], interaction: { action: string; args?: readonly number[] }): void {
  if (interaction.action !== "explode") throw new Error(`Unsupported top-down-friction dump action: ${interaction.action}`);
  const [x = 0, y = 10, z = 0, radius = 10, falloff = 5, impulsePerArea = 10000] = interaction.args ?? [];
  world.explode([x, y, z], radius, falloff, impulsePerArea, 0xFFFFFFFFn as unknown as number);
}

export const dumpCreate = createTopDownFriction;
