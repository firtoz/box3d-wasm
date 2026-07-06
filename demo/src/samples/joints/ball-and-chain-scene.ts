import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";

export function createBallAndChain(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyHandle[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0] });
  const objectWorld = ObjectRuntime.fromRuntime(runtime).wrapWorld(world);
  const ground = objectWorld.createBody();
  const handles: BodyHandle[] = [ground.handle];

  const linkRadius = 0.125;
  const linkExtent = 0.5;
  let parent = ground;
  for (let i = 0; i < 32; i++) {
    const child = objectWorld.createBody({ type: BodyType.Dynamic, position: [(1 + 2 * i) * linkExtent, 0, 0] });
    child.createCapsuleShape([-linkExtent, 0, 0], [linkExtent, 0, 0], linkRadius);
    objectWorld.createSphericalJoint(parent, child, {
      localFrameA: { position: parent === ground ? [0, 0, 0] : [linkExtent, 0, 0] },
      localFrameB: { position: [-linkExtent, 0, 0] },
      enableMotor: true,
      maxMotorTorque: 10,
      motorVelocity: [0, 0, 0],
    });
    handles.push(child.handle);
    parent = child;
  }

  const sphereRadius = 2;
  const ball = objectWorld.createBody({ type: BodyType.Dynamic, position: [(1 + 2 * 32) * linkExtent + sphereRadius - linkExtent, 0, 0] });
  ball.createSphereShape([0, 0, 0], sphereRadius);
  objectWorld.createSphericalJoint(parent, ball, {
    localFrameA: { position: [linkExtent, 0, 0] },
    localFrameB: { position: [-sphereRadius, 0, 0] },
    enableMotor: true,
    maxMotorTorque: 10,
    motorVelocity: [0, 0, 0],
  });
  handles.push(ball.handle);

  return { world, handles };
}

export const ballAndChainCamera = { position: [0, 15, 50] as [number, number, number], target: [0, -20, 0] as [number, number, number] };
export const dumpSampleName = "Ball and Chain";
export const dumpSampleId = "joints/ball-and-chain";
export const dumpCppSampleName = "Ball and Chain";
export const dumpCreate = createBallAndChain;
