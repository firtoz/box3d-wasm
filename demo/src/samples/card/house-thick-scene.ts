import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const B3_PI = 3.14159265359;
const B3_DEG_TO_RAD = 0.01745329251;
const ALPHA = 25 * B3_DEG_TO_RAD;
const CARD_HALF_DEPTH = 0.04;
const CARD_HALF_HEIGHT = 0.49;
const CARD_HALF_WIDTH = 0.19;

function addVerticalPair(
  world: PhysicsWorld, runtime: Box3DRuntime, handles: number[],
  startX: number, offsetX: number, startY: number,
): void {
  const sine = runtime.b3wSin(0.5 * ALPHA);
  const cosine = runtime.b3wCos(0.5 * ALPHA);
  for (const sign of [-1, 1]) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [startX + sign * offsetX, startY, 0], rotation: [0, 0, sign * sine, cosine], isAwake: true });
    runtime.createHullShape(body, [CARD_HALF_DEPTH, CARD_HALF_HEIGHT, CARD_HALF_WIDTH], { friction: 0.8 });
    handles.push(body);
  }
}

function addHorizontalRow(
  world: PhysicsWorld, runtime: Box3DRuntime, handles: number[],
  startX: number, offsetX: number, startY: number, count: number,
): void {
  const sine = runtime.b3wSin(0.25 * B3_PI);
  const cosine = runtime.b3wCos(0.25 * B3_PI);
  for (let i = 0; i < count; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [startX + i * offsetX, startY, 0], rotation: [0, 0, sine, cosine], isAwake: true });
    runtime.createHullShape(body, [CARD_HALF_DEPTH, CARD_HALF_HEIGHT, CARD_HALF_WIDTH], { friction: 0.8 });
    handles.push(body);
  }
}

function addVerticalRow(
  world: PhysicsWorld, runtime: Box3DRuntime, handles: number[],
  n: number, startX: number, offsetX: number, startY: number,
): void {
  for (let j = 0; j < n; j++) {
    addVerticalPair(world, runtime, handles, startX + j * 4 * offsetX, offsetX, startY);
  }
}

export function buildCardHouseThickDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const offsetX = 0.5 * 0.98 * runtime.b3wSin(ALPHA) + 0.045;
  const offsetY = 0.5 * 0.98 * runtime.b3wCos(ALPHA) + 0.035;

  addVerticalRow(world, runtime, handles, 4, -6 * offsetX, offsetX, offsetY);
  addHorizontalRow(world, runtime, handles, -4 * offsetX, 4 * offsetX, 2 * offsetY + 0.04, 3);
  addVerticalRow(world, runtime, handles, 3, -4 * offsetX, offsetX, 3 * offsetY + 0.08);
  addHorizontalRow(world, runtime, handles, -2 * offsetX, 4 * offsetX, 4 * offsetY + 0.12, 2);
  addVerticalRow(world, runtime, handles, 2, -2 * offsetX, offsetX, 5 * offsetY + 0.16);
  addHorizontalRow(world, runtime, handles, 0, 4 * offsetX, 6 * offsetY + 0.20, 1);
  addVerticalRow(world, runtime, handles, 1, 0, offsetX, 7 * offsetY + 0.24);

  return handles;
}

export function cardHouseThickGroundSize(): Vec3 {
  return [10, 1, 10];
}

function qz(angle: number): [number, number, number, number] { return [0, 0, Math.sin(angle / 2), Math.cos(angle / 2)]; }

export function createCardHouseThickBodies(): RenderBody[] {
  const alpha = 25 * Math.PI / 180;
  const ox = 0.5 * 0.98 * Math.sin(alpha) + 0.045;
  const oy = 0.5 * 0.98 * Math.cos(alpha) + 0.035;
  const bodies: RenderBody[] = [];
  const addPair = (x: number, y: number, count: number) => {
    for (let j = 0; j < count; j++) {
      for (const s of [-1, 1]) bodies.push({ kind: "box", size: [0.08, 0.98, 0.38], position: [x + s * ox, y, 0], rotation: qz(s * alpha), color: 0xfde68a });
      x += 4 * ox;
    }
  };
  const addRow = (x: number, y: number, c: number) => {
    for (let i = 0; i < c; i++) bodies.push({ kind: "box", size: [0.08, 0.98, 0.38], position: [x + i * 4 * ox, y, 0], rotation: qz(Math.PI / 2), color: 0xfde68a });
  };
  addPair(-6 * ox, oy, 4); addRow(-4 * ox, 2 * oy + 0.04, 3); addPair(-4 * ox, 3 * oy + 0.08, 3); addRow(-2 * ox, 4 * oy + 0.12, 2); addPair(-2 * ox, 5 * oy + 0.16, 2); addRow(0, 6 * oy + 0.20, 1); addPair(0, 7 * oy + 0.24, 1);
  return bodies;
}

export const cardHouseThickCamera: RenderSpec["camera"] = { position: [0, 6.226, 9.063], target: [0, 2, 0] };

export const dumpSampleName = "Card House Thick";
export const dumpSampleId = "card-house-thick";
export const dumpCppSampleName = "Card House Thick";
export const dumpGroundSize = cardHouseThickGroundSize;
export const dumpBuildDynamicBodies = buildCardHouseThickDynamicBodies;
