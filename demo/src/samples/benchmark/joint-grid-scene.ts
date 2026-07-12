import { BodyType, type BodyHandle, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const N = 100;

export const JOINT_GRID_SPHERE_COUNT = N * N;
export const JOINT_GRID_SPHERE_RADIUS = 0.4;
export const JOINT_GRID_DYNAMIC_COLOR = 0x60a5fa;
export const JOINT_GRID_STATIC_COLOR = 0x94a3b8;

export function forEachJointGridSphere(callback: (position: Vec3, color: number, isStatic: boolean) => void): void {
  for (let k = 0; k < N; k++) {
    for (let i = 0; i < N; i++) {
      const isStatic = i === 0;
      callback([k, -i, 0], isStatic ? JOINT_GRID_STATIC_COLOR : JOINT_GRID_DYNAMIC_COLOR, isStatic);
    }
  }
}

export function buildJointGridDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = new Array(N * N);
  let index = 0;

  for (let k = 0; k < N; k++) {
    for (let i = 0; i < N; i++) {
      const body = world.createBody({
        type: i === 0 ? BodyType.Static : BodyType.Dynamic,
        position: [k, -i, 0],
        enableSleep: false,
      });
      runtime.createSphereShape(body, [0, 0, 0], JOINT_GRID_SPHERE_RADIUS, {
        categoryBits: 2,
        maskBits: ~2 >>> 0,
      });
      handles[index] = body;

      if (i > 0) {
        world.createSphericalJoint(handles[index - 1]! as BodyHandle, body, {
          localFrameA: { position: [0, -0.5, 0] },
          localFrameB: { position: [0, 0.5, 0] },
        });
      }
      if (k > 0) {
        world.createSphericalJoint(handles[index - N]! as BodyHandle, body, {
          localFrameA: { position: [0.5, 0, 0] },
          localFrameB: { position: [-0.5, 0, 0] },
        });
      }

      index++;
    }
  }

  return handles;
}

export function jointGridGroundSize(): Vec3 { return [120, 1, 120]; }

export function createJointGridBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  forEachJointGridSphere((position, color, isStatic) => {
    bodies.push({
      kind: "sphere",
      radius: JOINT_GRID_SPHERE_RADIUS,
      position,
      color,
      type: isStatic ? BodyType.Static : BodyType.Dynamic,
    });
  });
  return bodies;
}

export const jointGridCamera: RenderSpec["camera"] = { position: [-25, 25, 94], target: [30, -30, 30] };

export const dumpSampleName = "Joint Grid";
export const dumpSampleId = "benchmark/joint-grid";
export const dumpCppSampleName = "Joint Grid";
export const dumpGroundSize = jointGridGroundSize;
export const dumpBuildDynamicBodies = buildJointGridDynamicBodies;

export function createJointGrid(runtime: Box3DRuntime): { world: PhysicsWorld; handles: number[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  world.enableSleeping(false);
  return { world, handles: buildJointGridDynamicBodies(world, runtime) };
}

export const dumpCreate = createJointGrid;
