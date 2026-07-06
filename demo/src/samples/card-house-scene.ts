import { B3_PI, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "./generic-host";

const cardHeight = 0.2;
const cardThickness = 0.001;
const cardDepth = 0.1;
const angle0 = 25.0 * B3_PI / 180.0;
const angle1 = -25.0 * B3_PI / 180.0;
const angle2 = 0.5 * B3_PI;
const f = Math.fround;

type CardDef = { position: Vec3; angle: number };

function createCardHouseDefs(): CardDef[] {
  const defs: CardDef[] = [];
  let nb = 5;
  let z0 = f(0.0);
  let y = f(cardHeight - 0.02);

  while (nb) {
    let z = z0;
    for (let i = 0; i < nb; i++) {
      if (i !== nb - 1) defs.push({ position: [f(z + 0.25), f(y + cardHeight - 0.015), 0], angle: angle2 });
      defs.push({ position: [z, y, 0], angle: angle1 });
      z = f(z + 0.175);
      defs.push({ position: [z, y, 0], angle: angle0 });
      z = f(z + 0.175);
    }
    y = f(y + (cardHeight * 2.0 - 0.03));
    z0 = f(z0 + 0.175);
    nb--;
  }

  return defs;
}

export function buildCardHouseDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];
  const q0: [number, number, number, number] = [0, 0, runtime.b3wSin(0.5 * angle0), runtime.b3wCos(0.5 * angle0)];
  const q1: [number, number, number, number] = [0, 0, runtime.b3wSin(0.5 * angle1), runtime.b3wCos(0.5 * angle1)];
  const q2: [number, number, number, number] = [0, 0, runtime.b3wSin(0.5 * angle2), runtime.b3wCos(0.5 * angle2)];
  for (const def of createCardHouseDefs()) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: def.position,
      rotation: def.angle === angle0 ? q0 : def.angle === angle1 ? q1 : q2,
    });
    runtime.createHullShape(body, [cardThickness, cardHeight, cardDepth], { friction: 0.7 });
    handles.push(body);
  }
  return handles;
}

export function cardHouseGroundSize(): Vec3 {
  return [10, 1, 10];
}

export function createCardHouseBodies(): RenderBody[] {
  return createCardHouseDefs().map((def) => ({
    kind: "box",
    size: [2 * cardThickness, 2 * cardHeight, 2 * cardDepth],
    position: def.position,
    rotation: [0, 0, Math.sin(def.angle / 2), Math.cos(def.angle / 2)],
    color: 0xfde68a,
    type: BodyType.Dynamic,
  }));
}

export const cardHouseCamera: RenderSpec["camera"] = { position: [30, 10, 3], target: [0.75, 1.0, 0.4] };

export const dumpSampleName = "Card House";
export const dumpSampleId = "card-house";
export const dumpCppSampleName = "Card House";
export const dumpGroundSize = cardHouseGroundSize;
export const dumpBuildDynamicBodies = buildCardHouseDynamicBodies;
