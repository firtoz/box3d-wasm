import { BodyType, type Box3DRuntime, type PhysicsWorld, type Quat, type Vec3 } from "box3d-wasm";

export function addBox(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: number[],
  position: Vec3,
  halfWidths: Vec3,
  rotation: Quat = [0, 0, 0, 1],
  options: { type?: BodyType; friction?: number; restitution?: number; rollingResistance?: number; density?: number; lock2d?: boolean; angularVelocity?: Vec3 } = {},
): number {
  const body = world.createBody({ type: options.type ?? BodyType.Dynamic, position, rotation, isAwake: true, angularVelocity: options.angularVelocity });
  runtime.createHullShape(body, halfWidths, { friction: options.friction, restitution: options.restitution, rollingResistance: options.rollingResistance, density: options.density });
  if (options.lock2d) runtime.setBodyMotionLocks(body, { lockLinearZ: true, lockRotationX: true, lockRotationY: true });
  handles.push(body);
  return body;
}
