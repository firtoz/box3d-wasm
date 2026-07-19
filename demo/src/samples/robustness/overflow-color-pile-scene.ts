import { BodyType, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";

const RING_COUNT = 5;
const PER_RING = 5;

// Float32 truncation helpers to match C++ float arithmetic exactly
const f32 = Math.fround;
const B3_PI_F32 = f32(Math.PI);
const TWO_PI_F32 = f32(2 * B3_PI_F32);

export function buildOverflowColorPileDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): number[] {
  const handles: number[] = [];

  // Tall heavy hub
  const hubHalfY = 2.5;
  const hub = world.createBody({
    type: BodyType.Dynamic, position: [0, hubHalfY, 0],
  });
  runtime.createHullShape(hub, [0.5, hubHalfY, 0.5], { density: 50 });
  handles.push(hub);

  // Neighbor rings around hub (positions computed with float32 precision
  // to match the upstream C++ sample which uses cosf/sinf from <math.h>)
  const neighborHalf = 0.2;
  const hubHalfX = 0.5;
  const ringRadius = hubHalfX + neighborHalf - 0.03;
  const ringSpacing = 0.5;
  const baseY = neighborHalf + 0.05;
  const ringRadiusF32 = f32(ringRadius);
  const ringSpacingF32 = f32(ringSpacing);
  const baseYF32 = f32(baseY);

  for (let ring = 0; ring < RING_COUNT; ring++) {
    const y = f32(baseYF32 + ringSpacingF32 * ring);
    const thetaOffset: number = (ring & 1) ? f32(B3_PI_F32 / PER_RING) : 0;

    for (let slot = 0; slot < PER_RING; slot++) {
      // Compute theta in float32 to match C++:
      // float theta = thetaOffset + (2.0f * B3_PI * slot) / OVERFLOW_PILE_PER_RING;
      const theta = f32(thetaOffset + f32(TWO_PI_F32 * slot / PER_RING));
      // cosf/sinf from math.h (not Box3D's Bhaskara approximation)
      const x = f32(ringRadiusF32 * runtime.b3wCosf(theta));
      const z = f32(ringRadiusF32 * runtime.b3wSinf(theta));
      const body = world.createBody({
        type: BodyType.Dynamic, position: [x, y, z],
      });
      runtime.createHullShape(body, [neighborHalf, neighborHalf, neighborHalf], {});
      handles.push(body);
    }
  }

  return handles;
}

export function overflowColorPileGroundSize(): Vec3 {
  return [20, 1, 20];
}

const R = RING_COUNT, P = PER_RING, nh = 0.2, rr = 0.5 + nh - 0.03, by = nh + 0.05;
const hubColor = 0xef4444;
const neighborColors = [0x3b82f6, 0x22c55e, 0xf59e0b, 0x8b5cf6, 0xec4899];
export const overflowColorPileBodies: RenderBody[] = [
  { kind: "box", size: [1, 5, 1], position: [0, 2.5, 0], color: hubColor },
  ...Array.from({ length: R * P }, (_, i) => {
    const ring = Math.floor(i / P);
    const slot = i % P;
    const theta = (ring & 1 ? Math.PI / P : 0) + (2 * Math.PI * slot) / P;
    return {
      kind: "box" as const, size: [2 * nh, 2 * nh, 2 * nh] as [number, number, number],
      position: [rr * Math.cos(theta), by + 0.5 * ring, rr * Math.sin(theta)] as [number, number, number],
      color: neighborColors[slot % neighborColors.length],
    };
  }),
];

export const overflowColorPileCamera: RenderSpec["camera"] = { position: [30, 35, 15], target: [0, 0, 0] };

export const dumpSampleName = "Overflow Color Pile";
export const dumpSampleId = "robustness/overflow-color-pile";
export const dumpCppSampleName = "Overflow Color Pile";
export const dumpGroundSize = overflowColorPileGroundSize;
export const dumpBuildDynamicBodies = buildOverflowColorPileDynamicBodies;
