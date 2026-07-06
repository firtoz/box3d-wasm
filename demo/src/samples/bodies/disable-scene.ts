import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { ObjectRuntime } from "box3d-wasm/objects";
import type { RenderBody, RenderSpec } from "../generic-host";

const linkRadius = 0.1;
const linkLength = 5 * linkRadius;
const e_count = 4;

export function buildDisableDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyHandle[] {
  const handles: BodyHandle[] = [];
  const objectRuntime = ObjectRuntime.fromRuntime(runtime);
  const objectWorld = objectRuntime.wrapWorld(world);
  const links = [];

  for (let link = 0; link < e_count; ++link) {
    const y = (e_count - link) * linkLength + 1.0;
    const body = objectWorld.createBody({
      type: link === 0 ? BodyType.Kinematic : BodyType.Dynamic,
      position: [0, y, 0],
    });
    body.createCapsuleShape([0, 0, 0], [0, -linkLength, 0], linkRadius);
    links.push(body);
    handles.push(body.handle);
  }

  for (let link = 0; link < e_count - 1; ++link) {
    objectWorld.createWeldJoint(links[link], links[link + 1], {
      localFrameA: { position: [0, -linkLength, 0] },
      angularHertz: 10,
      angularDampingRatio: 1,
    });
  }

  const ball = objectWorld.createBody({
    type: BodyType.Dynamic,
    position: [3, 3, 0],
  });
  ball.createSphereShape([0, 0, 0], 0.5);
  handles.push(ball.handle);

  return handles;
}

export function disableGroundSize(): Vec3 {
  return [20, 1, 20];
}

// handles[0..3] = links 0..3, handles[4] = ball (plus ground at handles[0] in dump)
// In the dump, handles = [ground(0), link0(1), link1(2), link2(3), link3(4), ball(5)]
// C++: m_bodyIds[2] is link 2 at handles[3]
export function stepDisable(
  _world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: readonly BodyHandle[],
  _frame: number,
  _dt: number,
): void {
  runtime.applyLinearImpulseToCenter(handles[3], [0, 0.1, 0], true);
}

export const disableBodies: RenderBody[] = [
  { kind: "capsule", radius: 0.1, length: 0.5, axis: "y", position: [0, 3, 0], localPosition: [0, -0.25, 0], type: BodyType.Kinematic, color: 0x3b82f6 },
  { kind: "capsule", radius: 0.1, length: 0.5, axis: "y", position: [0, 2.5, 0], localPosition: [0, -0.25, 0], type: BodyType.Dynamic, color: 0x22c55e },
  { kind: "capsule", radius: 0.1, length: 0.5, axis: "y", position: [0, 2, 0], localPosition: [0, -0.25, 0], type: BodyType.Dynamic, color: 0xf97316 },
  { kind: "capsule", radius: 0.1, length: 0.5, axis: "y", position: [0, 1.5, 0], localPosition: [0, -0.25, 0], type: BodyType.Dynamic, color: 0xef4444 },
  { kind: "sphere", radius: 0.5, position: [3, 3, 0], type: BodyType.Dynamic, color: 0xf59e0b },
];

export const disableCamera: RenderSpec["camera"] = { position: [45, 25, 10], target: [0, 0, 0] };

export const dumpSampleName = "Disable";
export const dumpSampleId = "bodies/disable";
export const dumpCppSampleName = "Disable";
export const dumpGroundSize = disableGroundSize;
export const dumpBuildDynamicBodies = buildDisableDynamicBodies;
export const dumpStep = stepDisable;
