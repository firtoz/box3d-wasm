import {B3_AXIS_Z, B3_PI, BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3, type BodyId} from "box3d-wasm";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "./f32";
import type { RenderBody, RenderSpec } from "./generic-host";

// Match upstream CardHouse float literals (`0.2f`, `0.001f`, …).
const cardHeight = f32(0.2);
const cardThickness = f32(0.001);
const cardDepth = f32(0.1);
const angle0 = f32Div(f32Mul(25, B3_PI), 180);
const angle1 = f32(-angle0);
const angle2 = f32Mul(0.5, B3_PI);
const stepZ = f32(0.175);
const roofXOffset = f32(0.25);
const roofYOffset = f32(0.015);
const rowYStep = f32Sub(f32Mul(cardHeight, 2), f32(0.03));

type CardDef = { position: Vec3; angle: number };

function createCardHouseDefs(): CardDef[] {
  const defs: CardDef[] = [];
  let nb = 5;
  let z0 = f32(0);
  // float: y = cardHeight - 0.02f
  let y = f32Sub(cardHeight, f32(0.02));

  while (nb) {
    let z = z0;
    for (let i = 0; i < nb; i++) {
      if (i !== nb - 1) {
        // float: { z + 0.25f, y + cardHeight - 0.015f, 0 }
        defs.push({
          position: [f32Add(z, roofXOffset), f32Sub(f32Add(y, cardHeight), roofYOffset), 0],
          angle: angle2,
        });
      }
      defs.push({ position: [z, y, 0], angle: angle1 });
      z = f32Add(z, stepZ);
      defs.push({ position: [z, y, 0], angle: angle0 });
      z = f32Add(z, stepZ);
    }
    // float: y += cardHeight * 2.0f - 0.03f; z0 += 0.175f
    y = f32Add(y, rowYStep);
    z0 = f32Add(z0, stepZ);
    nb--;
  }

  return defs;
}

export function buildCardHouseDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const handles: BodyId[] = [];
  const q0 = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, angle0);
  const q1 = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, angle1);
  const q2 = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, angle2);
  for (const def of createCardHouseDefs()) {
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: def.position,
      rotation: def.angle === angle0 ? q0 : def.angle === angle1 ? q1 : q2,
    });
    // Upstream: b3MakeBoxHull + b3CreateHullShape with default density; friction/rollingResistance only.
    runtime.createHullShape(body, [cardThickness, cardHeight, cardDepth], { friction: 0.7, rollingResistance: 0.05 });
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
