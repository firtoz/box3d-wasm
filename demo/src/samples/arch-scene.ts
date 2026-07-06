import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

const halfDepth = 0.5;

const ps1: Vec3[] = [
  [16.0 * 0.25, 0.0 * 0.25, 0],
  [14.93803712795643 * 0.25, 5.133601056842984 * 0.25, 0],
  [13.79871746027416 * 0.25, 10.24928069555078 * 0.25, 0],
  [12.56252963284711 * 0.25, 15.34107019122473 * 0.25, 0],
  [11.20040987372525 * 0.25, 20.39856541571217 * 0.25, 0],
  [9.66521217819836 * 0.25, 25.40369899225096 * 0.25, 0],
  [7.87179930638133 * 0.25, 30.3179337000085 * 0.25, 0],
  [5.635199558196225 * 0.25, 35.03820717801641 * 0.25, 0],
  [2.405937953536585 * 0.25, 39.09554102558315 * 0.25, 0],
];

const ps2: Vec3[] = [
  [24.0 * 0.25, 0.0 * 0.25, 0],
  [22.33619528222415 * 0.25, 6.02299846205841 * 0.25, 0],
  [20.54936888969905 * 0.25, 12.00964361211476 * 0.25, 0],
  [18.60854610798073 * 0.25, 17.9470321677465 * 0.25, 0],
  [16.46769273811807 * 0.25, 23.81367936585418 * 0.25, 0],
  [14.05325025774858 * 0.25, 29.57079353071012 * 0.25, 0],
  [11.23551045834022 * 0.25, 35.13775818285372 * 0.25, 0],
  [7.752568160730571 * 0.25, 40.30450679009583 * 0.25, 0],
  [3.016931552701656 * 0.25, 44.28891593799322 * 0.25, 0],
];

function archSegmentPoints(i: number): Vec3[] {
  return [
    [ps1[i][0], ps1[i][1], -halfDepth],
    [ps2[i][0], ps2[i][1], -halfDepth],
    [ps2[i + 1][0], ps2[i + 1][1], -halfDepth],
    [ps1[i + 1][0], ps1[i + 1][1], -halfDepth],
    [ps1[i][0], ps1[i][1], halfDepth],
    [ps2[i][0], ps2[i][1], halfDepth],
    [ps2[i + 1][0], ps2[i + 1][1], halfDepth],
    [ps1[i + 1][0], ps1[i + 1][1], halfDepth],
  ];
}

function archMirrorSegmentPoints(i: number): Vec3[] {
  return [
    [-ps2[i][0], ps2[i][1], -halfDepth],
    [-ps1[i][0], ps1[i][1], -halfDepth],
    [-ps1[i + 1][0], ps1[i + 1][1], -halfDepth],
    [-ps2[i + 1][0], ps2[i + 1][1], -halfDepth],
    [-ps2[i][0], ps2[i][1], halfDepth],
    [-ps1[i][0], ps1[i][1], halfDepth],
    [-ps1[i + 1][0], ps1[i + 1][1], halfDepth],
    [-ps2[i + 1][0], ps2[i + 1][1], halfDepth],
  ];
}

function archCapPoints(): Vec3[] {
  return [
    [ps1[8][0], ps1[8][1], -halfDepth],
    [ps2[8][0], ps2[8][1], -halfDepth],
    [-ps2[8][0], ps2[8][1], -halfDepth],
    [-ps1[8][0], ps1[8][1], -halfDepth],
    [ps1[8][0], ps1[8][1], halfDepth],
    [ps2[8][0], ps2[8][1], halfDepth],
    [-ps2[8][0], ps2[8][1], halfDepth],
    [-ps1[8][0], ps1[8][1], halfDepth],
  ];
}

export function buildArchDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  for (let i = 0; i < 8; i++) {
    const hull = runtime.createHullFromPoints(archSegmentPoints(i).flat());
    const body = world.createBody({ type: BodyType.Dynamic });
    runtime.createShapeFromHull(body, hull, { density: 200 });
    runtime.destroyHull(hull);
    handles.push(body);
  }

  for (let i = 0; i < 8; i++) {
    const hull = runtime.createHullFromPoints(archMirrorSegmentPoints(i).flat());
    const body = world.createBody({ type: BodyType.Dynamic });
    runtime.createShapeFromHull(body, hull, { density: 200 });
    runtime.destroyHull(hull);
    handles.push(body);
  }

  {
    const hull = runtime.createHullFromPoints(archCapPoints().flat());
    const body = world.createBody({ type: BodyType.Dynamic });
    runtime.createShapeFromHull(body, hull, { density: 200 });
    runtime.destroyHull(hull);
    handles.push(body);
  }

  for (let i = 0; i < 4; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [0, 0.5 + ps2[8][1] + i, 0] });
    runtime.createHullShape(body, [2.0, 0.5, halfDepth], { density: 200 });
    handles.push(body);
  }

  return handles;
}

export function archGroundSize(): Vec3 {
  return [40, 1, 40];
}

export function createArchBodies(): RenderBody[] {
  const bodies: RenderBody[] = [];
  for (let i = 0; i < 8; i++) bodies.push({ kind: "hull", points: archSegmentPoints(i), position: [0, 0, 0], color: 0x38bdf8, type: BodyType.Dynamic });
  for (let i = 0; i < 8; i++) bodies.push({ kind: "hull", points: archMirrorSegmentPoints(i), position: [0, 0, 0], color: 0x38bdf8, type: BodyType.Dynamic });
  bodies.push({ kind: "hull", points: archCapPoints(), position: [0, 0, 0], color: 0x38bdf8, type: BodyType.Dynamic });
  for (let i = 0; i < 4; i++) bodies.push({ kind: "box", size: [4, 1, 1], position: [0, 0.5 + ps2[8][1] + i, 0], color: 0x38bdf8, type: BodyType.Dynamic });
  return bodies;
}

export const archCamera: RenderSpec["camera"] = { position: [25, 10, 30], target: [0, 5, 0] };

export const dumpSampleName = "Arch";
export const dumpSampleId = "arch";
export const dumpCppSampleName = "Arch";
export const dumpGroundSize = archGroundSize;
export const dumpBuildDynamicBodies = buildArchDynamicBodies;
