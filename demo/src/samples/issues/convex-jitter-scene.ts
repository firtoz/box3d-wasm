import type { Box3DRuntime, PhysicsWorld, Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

function scalePoints(points: number[][], s: number): number[] {
  const out: number[] = [];
  for (const p of points) {
    out.push(p[0] * s, p[2] * s, p[1] * s);
  }
  return out;
}

export function buildConvexJitterDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const s = 0.01;

  // Body 1: static hull with convex jitter points
  {
    const raw = [
      [-44.8770714, -91.6598053, -1.92012548],
      [-92.5001831, 51.0151291, 15.8006573],
      [-91.0282211, -9.44371605, 15.6148796],
      [90.2375641, 77.3870087, 15.9356089],
      [-85.5353241, 91.3750992, -1.36629653],
      [88.9092178, -87.2975464, -1.86754704],
      [83.7932816, -89.8572235, 15.4168339],
      [87.0243988, 88.9776535, -1.32423306],
      [-91.6564941, -85.4949493, 15.3782759],
      [-90.2922516, -87.2074127, -1.92012548],
      [-87.2944870, 89.9510498, 15.9215889],
      [79.2338104, 89.9690781, 15.9724140],
      [-91.6744461, 81.0823212, -1.39959598],
      [90.3452759, -76.4459610, 15.4588966],
      [-87.4021912, -89.2263107, 15.3677588],
      [76.3258057, 92.0059967, 1.82873762],
    ];
    const b: [number, number, number] = [-459.292877, 217.398331, 1.00115335];
    const hull = runtime.createHullFromPoints(scalePoints(raw, s));
    const body = world.createBody({
      type: 0 as const,
      position: [s * b[0], s * b[2] + 2, s * b[1]],
      rotation: [0, -0.707106769, 0, 0.707106769],
    });
    runtime.createShapeFromHull(body, hull, {});
    runtime.destroyHull(hull);
    handles.push(body);
  }

  // Body 2: dynamic hull with rolling resistance
  {
    const raw = [
      [29.5000000, 17.1488495, 0.175081104],
      [29.5000000, -17.2990532, 0.125000000],
      [29.4840164, -17.3057766, 24.0200863],
      [29.4840164, 17.1648350, 24.1781254],
      [-29.1345520, 17.5529804, 0.125000000],
      [-29.1345520, 17.5529804, 23.7899799],
      [-29.1441040, 16.9679585, 24.3750000],
      [-29.1345520, -17.2990532, 24.3750000],
      [-29.1345520, -17.2990532, 0.175081253],
      [29.0720215, 17.5529785, 0.125000000],
      [29.0859070, 17.5629406, 23.8120594],
      [29.1401348, -17.2990532, 24.3750000],
      [29.1123581, 16.9722290, 24.4027710],
      [29.3944912, 17.2543602, 24.1206398],
      [-29.1345520, -17.2990532, 24.0759430],
      [-29.1345520, -16.9722252, 24.4027710],
      [29.1123619, -16.9722271, 24.4027729],
      [29.5000000, 17.3429642, 24.0000000],
    ];
    const b: [number, number, number] = [-402.321838, 157.310364, 16.816925];
    const hull = runtime.createHullFromPoints(scalePoints(raw, s));
    const body = world.createBody({
      type: 2 as const,
      position: [s * b[0], s * b[2] + 2, s * b[1]],
      rotation: [0, -0.00152086187, 0, 0.999998868],
    });
    runtime.createShapeFromHull(body, hull, { rollingResistance: 0.1 });
    runtime.destroyHull(hull);
    handles.push(body);
  }

  return handles;
}

export function convexJitterGroundSize(): Vec3 {
  return [5, 1, 5];
}

const _s = 0.01;
const _b1: [number, number, number] = [-459.292877, 217.398331, 1.00115335];
const _b2: [number, number, number] = [-402.321838, 157.310364, 16.816925];
export const convexJitterBodies: RenderBody[] = [
  { kind: "box", size: [0.5, 0.5, 0.5], position: [_s * _b1[0], _s * _b1[2] + 2, _s * _b1[1]], color: 0x3b82f6 },
  { kind: "box", size: [0.5, 0.5, 0.5], position: [_s * _b2[0], _s * _b2[2] + 2, _s * _b2[1]], color: 0xf59e0b },
];

export const convexJitterCamera: RenderSpec["camera"] = { position: [0, 15, 10], target: [0, 2, 0] };

export const dumpSampleName = "Convex Jitter";
export const dumpSampleId = "issues/convex-jitter";
export const dumpCppSampleName = "Convex Jitter";
export const dumpGroundSize = convexJitterGroundSize;
export const dumpBuildDynamicBodies = buildConvexJitterDynamicBodies;
