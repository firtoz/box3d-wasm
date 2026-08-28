import {
  B3_AXIS_X,
  B3_AXIS_Y,
  B3_AXIS_Z,
  B3_PI,
  BodyType,
  type BodyId,
  type Box3DRuntime,
  type HullHandle,
  type JointId,
  type MeshHandle,
  type PhysicsWorld,
  type Quat,
  type Vec3,
} from "box3d-wasm";
import { Box3DRng } from "../box3d-rng";
import { f32, f32Add, f32Div, f32Mul, f32Sub } from "../f32";
import type { RenderBody, RenderPart } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const GEAR_RADIUS = 1;
export const GEAR_HALF_DEPTH = 0.125;
export const GEAR_Z = 1.5;
export const AXLE_RADIUS = 0.2;
export const TOOTH_HALF_WIDTH = 0.11;
export const TOOTH_HALF_HEIGHT = 0.09;
export const TOOTH_RADIUS = 0.03;
export const LINK_HALF_LENGTH = 0.07;
export const LINK_RADIUS = 0.05;
export const LINK_COUNT = 40;
export const DOOR_HALF_HEIGHT = 1.5;
export const DOOR_HALF_DEPTH = 1.95;
export const GEAR_SIDES = 24;
export const AXLE_SIDES = 12;
export const ROCK_RADIUS = 0.3;
export const DEBRIS_X_COUNT = 12;
export const DEBRIS_Y_COUNT = 10;

export const GEAR_LIFT_MOTOR_TORQUE = 30000;
export const GEAR_LIFT_MOTOR_SPEED = -0.3;

const COLOR_STAIR = 0x8fbc8f;
const COLOR_DISK = 0x8b4513;
const COLOR_AXLE = 0x708090;
const COLOR_TOOTH = 0x808080;
const COLOR_LINK = 0xb0c4de;
const COLOR_DOOR = 0x008b8b;
const COLOR_ROCKS = [0x808080, 0xdcdcdc, 0xd3d3d3, 0x778899, 0xa9a9a9] as const;
const DOOR_DEBUG_COLOR = (5 << 24) | COLOR_DOOR;

const SILHOUETTE: readonly [number, number][] = [
  [-11.3, -0.2167], [9.3375, -0.2167], [9.3375, 7.1917], [8.8083, 7.1917], [8.8083, 0.3125],
  [0.3417, 0.3125], [0.3417, 0.8417], [-0.1875, 0.8417], [-0.1875, 1.3708], [-0.7167, 1.3708],
  [-0.7167, 1.9], [-1.2458, 1.9], [-1.2458, 2.4292], [-1.775, 2.4292], [-1.775, 2.9583],
  [-2.3042, 2.9583], [-2.3042, 3.4875], [-2.8333, 3.4875], [-2.8333, 4.0167], [-3.3625, 4.0167],
  [-3.3625, 4.5458], [-3.8917, 4.5458], [-3.8917, 5.075], [-4.4208, 5.075], [-4.4208, 5.6042],
  [-4.95, 5.6042], [-4.95, 6.1333], [-5.4792, 6.1333], [-5.4792, 6.6625], [-6.0083, 6.6625],
  [-6.0083, 7.1917], [-11.3, 7.1917],
];

/** Mapbox earcut of `SILHOUETTE` (same source as upstream `earcut.h` / Three.js Earcut). */
const CAP_INDICES = [
  30, 31, 0, 1, 2, 3, 5, 6, 7, 7, 8, 9, 9, 10, 11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 17, 18, 19, 19, 20, 21, 21, 22, 23,
  23, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 0, 1, 3, 4, 5, 7, 9, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 0, 0, 1, 4,
  9, 13, 15, 15, 19, 21, 21, 25, 27, 0, 4, 5, 9, 15, 21, 21, 27, 0, 0, 5, 9, 9, 21, 0,
] as const;

const Z_MIN = -2;
const Z_MAX = 2;
const WALL_HALF_THICK = 0.05;

const GEAR_POS_1: Vec3 = [-4.25, 9.75, 0];
const GEAR_POS_2: Vec3 = [-2.25, 10.75, 0];

export function gearLiftGroundSize(): Vec3 {
  return [20, 1, 20];
}

export const gearLiftCamera = cameraFromSetView(18, 12, 17, [-1.5, 4.5, 0]);

export function gearLiftStairMesh(): { vertices: number[]; indices: number[] } {
  const vertices: number[] = [];
  for (const [x, y] of SILHOUETTE) {
    vertices.push(x, y, Z_MIN, x, y, Z_MAX);
  }
  const indices: number[] = [];
  for (let i = 0; i < 32; i++) {
    const j = (i + 1) % 32;
    const aLo = 2 * i;
    const aHi = 2 * i + 1;
    const bLo = 2 * j;
    const bHi = 2 * j + 1;
    indices.push(aLo, bLo, bHi, aLo, bHi, aHi);
  }
  for (let k = 0; k + 3 <= CAP_INDICES.length; k += 3) {
    pushCap(indices, CAP_INDICES[k]!, CAP_INDICES[k + 1]!, CAP_INDICES[k + 2]!, 1, true);
    pushCap(indices, CAP_INDICES[k]!, CAP_INDICES[k + 1]!, CAP_INDICES[k + 2]!, 0, false);
  }
  return { vertices, indices };
}

function pushCap(indices: number[], r0: number, r1: number, r2: number, vOffset: number, wantPositiveZ: boolean): void {
  const p0 = SILHOUETTE[r0]!;
  const p1 = SILHOUETTE[r1]!;
  const p2 = SILHOUETTE[r2]!;
  const cross = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p1[1] - p0[1]) * (p2[0] - p0[0]);
  const positive = cross > 0;
  const v0 = 2 * r0 + vOffset;
  const v1 = 2 * r1 + vOffset;
  const v2 = 2 * r2 + vOffset;
  indices.push(v0);
  if (positive === wantPositiveZ) indices.push(v1, v2);
  else indices.push(v2, v1);
}

function wallBox(): { center: Vec3; half: Vec3 } {
  let lowerX = SILHOUETTE[0]![0];
  let lowerY = SILHOUETTE[0]![1];
  let upperX = lowerX;
  let upperY = lowerY;
  for (const [x, y] of SILHOUETTE) {
    lowerX = Math.min(lowerX, x);
    lowerY = Math.min(lowerY, y);
    upperX = Math.max(upperX, x);
    upperY = Math.max(upperY, y);
  }
  return {
    center: [
      f32Mul(0.5, f32Add(lowerX, upperX)),
      f32Mul(0.5, f32Add(lowerY, upperY)),
      f32Sub(-Z_MAX, WALL_HALF_THICK),
    ],
    half: [
      f32Mul(0.5, f32Sub(upperX, lowerX)),
      f32Mul(0.5, f32Sub(upperY, lowerY)),
      WALL_HALF_THICK,
    ],
  };
}

function zCylinderPoints(runtime: Box3DRuntime, radius: number, zMin: number, zMax: number, sides: number): number[] {
  const points: number[] = [];
  for (let i = 0; i < sides; i++) {
    const angle = f32Div(f32Mul(f32Mul(2, B3_PI), i), sides);
    const x = f32Mul(radius, runtime.b3wCosf(angle));
    const y = f32Mul(radius, runtime.b3wSinf(angle));
    points.push(x, y, zMin, x, y, zMax);
  }
  return points;
}

function pointsToTuples(points: number[]): [number, number, number][] {
  const tuples: [number, number, number][] = [];
  for (let i = 0; i < points.length; i += 3) tuples.push([points[i]!, points[i + 1]!, points[i + 2]!]);
  return tuples;
}

function addVec(a: Vec3, b: Vec3): Vec3 {
  return [f32Add(a[0], b[0]), f32Add(a[1], b[1]), f32Add(a[2], b[2])];
}

function toothPoints(runtime: Box3DRuntime, centerRadius: number, zCenter: number, index: number): number[] {
  const deltaAngle = f32Div(f32Mul(2, B3_PI), 16);
  const q = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, f32Mul(index, deltaAngle));
  const rotated = runtime.rotateVector(q, [centerRadius, 0, 0]);
  const center: Vec3 = [rotated[0], rotated[1], zCenter];
  const hx = TOOTH_HALF_WIDTH;
  const hz = GEAR_HALF_DEPTH;
  const baseHalf = TOOTH_HALF_HEIGHT;
  const tipHalf = f32Sub(TOOTH_HALF_HEIGHT, TOOTH_RADIUS);
  const local: Vec3[] = [
    [-hx, -baseHalf, -hz], [-hx, baseHalf, -hz], [-hx, baseHalf, hz], [-hx, -baseHalf, hz],
    [hx, -tipHalf, -hz], [hx, tipHalf, -hz], [hx, tipHalf, hz], [hx, -tipHalf, hz],
  ];
  const points: number[] = [];
  for (const p of local) {
    const world = addVec(center, runtime.rotateVector(q, p));
    points.push(world[0], world[1], world[2]);
  }
  return points;
}

function gearParts(runtime: Box3DRuntime, toothCenterRadius: number): [RenderPart, ...RenderPart[]] {
  const diskNear = zCylinderPoints(runtime, GEAR_RADIUS, f32Sub(-GEAR_Z, GEAR_HALF_DEPTH), f32Add(-GEAR_Z, GEAR_HALF_DEPTH), GEAR_SIDES);
  const diskFar = zCylinderPoints(runtime, GEAR_RADIUS, f32Sub(GEAR_Z, GEAR_HALF_DEPTH), f32Add(GEAR_Z, GEAR_HALF_DEPTH), GEAR_SIDES);
  const axle = zCylinderPoints(runtime, AXLE_RADIUS, -GEAR_Z, GEAR_Z, AXLE_SIDES);
  const parts: RenderPart[] = [
    { kind: "hull", points: pointsToTuples(diskNear), color: COLOR_DISK },
    { kind: "hull", points: pointsToTuples(diskFar), color: COLOR_DISK },
    { kind: "hull", points: pointsToTuples(axle), color: COLOR_AXLE },
  ];
  for (const z of [-GEAR_Z, GEAR_Z]) {
    for (let i = 0; i < 16; i++) {
      parts.push({ kind: "hull", points: pointsToTuples(toothPoints(runtime, toothCenterRadius, z, i)), color: COLOR_TOOTH });
    }
  }
  return parts as [RenderPart, ...RenderPart[]];
}

export interface GearLiftScene {
  handles: BodyId[];
  driverJoint: JointId;
  mesh: MeshHandle;
}

function createStairwell(world: PhysicsWorld): { body: BodyId; mesh: MeshHandle } {
  const body = world.createBody({ type: BodyType.Static });
  const { vertices, indices } = gearLiftStairMesh();
  const mesh = world.createMesh(vertices, indices, { useMedianSplit: false, identifyEdges: true });
  world.createMeshShape(body, mesh, { customColor: COLOR_STAIR });
  const wall = wallBox();
  world.createOffsetHullShape(body, wall.half, wall.center, { customColor: COLOR_STAIR });
  return { body, mesh };
}

function buildGearBody(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  position: Vec3,
  toothCenterRadius: number,
  diskNear: HullHandle,
  diskFar: HullHandle,
  axle: HullHandle,
): BodyId {
  const body = world.createBody({ type: BodyType.Dynamic, position });
  const diskDef = { friction: 0.1, customColor: COLOR_DISK };
  runtime.createShapeFromHull(body, diskNear, diskDef);
  runtime.createShapeFromHull(body, diskFar, diskDef);
  runtime.createShapeFromHull(body, axle, { friction: 0.1, customColor: COLOR_AXLE });
  for (const z of [-GEAR_Z, GEAR_Z]) {
    for (let i = 0; i < 16; i++) {
      const tooth = runtime.createHullFromPoints(toothPoints(runtime, toothCenterRadius, z, i));
      runtime.createShapeFromHull(body, tooth, { friction: 0.1, customColor: COLOR_TOOTH });
      runtime.destroyHull(tooth);
    }
  }
  return body;
}

function createChain(world: PhysicsWorld, runtime: Box3DRuntime, topBody: BodyId, attach: Vec3): { last: BodyId; handles: BodyId[] } {
  const center1: Vec3 = [0, -LINK_HALF_LENGTH, 0];
  const center2: Vec3 = [0, LINK_HALF_LENGTH, 0];
  const position: Vec3 = [attach[0], f32Sub(attach[1], LINK_HALF_LENGTH), attach[2]];
  let prev = topBody;
  const handles: BodyId[] = [];
  for (let i = 0; i < LINK_COUNT; i++) {
    const body = world.createBody({ type: BodyType.Dynamic, position: [position[0], position[1], position[2]] });
    runtime.createCapsuleShape(body, center1, center2, LINK_RADIUS, { customColor: COLOR_LINK });
    const pivot: Vec3 = [position[0], f32Add(position[1], LINK_HALF_LENGTH), attach[2]];
    world.createRevoluteJoint(prev, body, {
      localFrameA: { position: world.getBodyLocalPoint(prev, pivot) },
      localFrameB: { position: world.getBodyLocalPoint(body, pivot) },
      enableMotor: true,
      maxMotorTorque: 0.05,
    });
    position[1] = f32Sub(position[1], f32Mul(2, LINK_HALF_LENGTH));
    prev = body;
    handles.push(body);
  }
  return { last: prev, handles };
}

function createDoor(world: PhysicsWorld, runtime: Box3DRuntime, groundId: BodyId, doorPosition: Vec3, nearLink: BodyId, farLink: BodyId): BodyId {
  const door = world.createBody({ type: BodyType.Dynamic, position: doorPosition });
  runtime.createHullShape(door, [0.05, DOOR_HALF_HEIGHT, DOOR_HALF_DEPTH], {
    density: 500,
    friction: 0.1,
    customColor: DOOR_DEBUG_COLOR,
  });
  const links = [nearLink, farLink];
  const depths = [-GEAR_Z, GEAR_Z];
  for (let i = 0; i < 2; i++) {
    const pivot: Vec3 = [doorPosition[0], f32Add(doorPosition[1], DOOR_HALF_HEIGHT), depths[i]!];
    world.createRevoluteJoint(links[i]!, door, {
      localFrameA: { position: world.getBodyLocalPoint(links[i]!, pivot) },
      localFrameB: { position: [0, DOOR_HALF_HEIGHT, depths[i]!] },
      enableMotor: true,
      maxMotorTorque: 50,
    });
  }
  const slideAxis = runtime.computeQuatBetweenUnitVectors(B3_AXIS_X, B3_AXIS_Y);
  world.createPrismaticJoint(groundId, door, {
    localFrameA: { position: world.getBodyLocalPoint(groundId, doorPosition), rotation: slideAxis },
    localFrameB: { position: [0, 0, 0], rotation: slideAxis },
    maxMotorForce: 200,
    enableMotor: true,
    collideConnected: true,
  });
  return door;
}

function randomQuat(runtime: Box3DRuntime, rng: Box3DRng): Quat {
  const u1 = rng.randomFloatRange(0, 1);
  const u2 = rng.randomFloatRange(0, 2 * B3_PI);
  const u3 = rng.randomFloatRange(0, 2 * B3_PI);
  const sqrt1MinusU1 = f32(Math.sqrt(f32Sub(1, u1)));
  const sqrtU1 = f32(Math.sqrt(u1));
  return [
    f32Mul(sqrt1MinusU1, runtime.b3wSin(u2)),
    f32Mul(sqrt1MinusU1, runtime.b3wCos(u2)),
    f32Mul(sqrtU1, runtime.b3wSin(u3)),
    f32Mul(sqrtU1, runtime.b3wCos(u3)),
  ];
}

function forEachDebrisPose(runtime: Box3DRuntime, rng: Box3DRng, callback: (position: Vec3, rotation: Quat, color: number) => void): void {
  let x = f32(-5);
  for (let i = 0; i < DEBRIS_X_COUNT; i++) {
    let y = f32Sub(f32(6.5), f32Mul(f32(0.25), i));
    for (let j = 0; j < DEBRIS_Y_COUNT; j++) {
      const z = rng.randomFloatRange(-1.65, 0.35);
      const rotation = randomQuat(runtime, rng);
      const color = COLOR_ROCKS[rng.randomIntRange(0, 4)]!;
      callback([x, y, z], rotation, color);
      y = f32Add(y, f32(0.2));
    }
    x = f32Add(x, f32(0.3));
  }
}

function createDebris(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const rockHull = runtime.createRock(ROCK_RADIUS);
  const handles: BodyId[] = [];
  const rng = new Box3DRng();
  forEachDebrisPose(runtime, rng, (position, rotation, color) => {
    const body = world.createBody({ type: BodyType.Dynamic, position, rotation });
    runtime.createShapeFromHull(body, rockHull, { rollingResistance: 0.3, customColor: color });
    handles.push(body);
  });
  runtime.destroyHull(rockHull);
  return handles;
}

export function buildGearLiftScene(world: PhysicsWorld, runtime: Box3DRuntime): GearLiftScene {
  const stair = createStairwell(world);
  const diskNear = runtime.createHullFromPoints(
    zCylinderPoints(runtime, GEAR_RADIUS, f32Sub(-GEAR_Z, GEAR_HALF_DEPTH), f32Add(-GEAR_Z, GEAR_HALF_DEPTH), GEAR_SIDES),
  );
  const diskFar = runtime.createHullFromPoints(
    zCylinderPoints(runtime, GEAR_RADIUS, f32Sub(GEAR_Z, GEAR_HALF_DEPTH), f32Add(GEAR_Z, GEAR_HALF_DEPTH), GEAR_SIDES),
  );
  const axle = runtime.createHullFromPoints(zCylinderPoints(runtime, AXLE_RADIUS, -GEAR_Z, GEAR_Z, AXLE_SIDES));
  const driverRadius = f32Add(GEAR_RADIUS, TOOTH_HALF_HEIGHT);
  const followerRadius = f32Add(GEAR_RADIUS, TOOTH_HALF_WIDTH);
  const driver = buildGearBody(world, runtime, GEAR_POS_1, driverRadius, diskNear, diskFar, axle);
  const follower = buildGearBody(world, runtime, GEAR_POS_2, followerRadius, diskNear, diskFar, axle);
  runtime.destroyHull(diskNear);
  runtime.destroyHull(diskFar);
  runtime.destroyHull(axle);

  const driverJoint = world.createRevoluteJoint(stair.body, driver, {
    localFrameA: { position: world.getBodyLocalPoint(stair.body, GEAR_POS_1) },
    localFrameB: { position: [0, 0, 0] },
    enableMotor: true,
    maxMotorTorque: GEAR_LIFT_MOTOR_TORQUE,
    motorSpeed: GEAR_LIFT_MOTOR_SPEED,
  });
  const followerFrame: Quat = runtime.makeQuatFromAxisAngle(B3_AXIS_Z, f32Mul(0.25, B3_PI));
  world.createRevoluteJoint(stair.body, follower, {
    localFrameA: { position: world.getBodyLocalPoint(stair.body, GEAR_POS_2), rotation: followerFrame },
    localFrameB: { position: [0, 0, 0] },
    enableMotor: true,
    maxMotorTorque: 0.5,
    enableLimit: true,
    lowerAngle: f32Mul(-0.3, B3_PI),
    upperAngle: f32Mul(0.8, B3_PI),
  });

  const linkAttachX = f32Add(f32Add(f32Add(GEAR_POS_2[0], GEAR_RADIUS), f32Mul(2, TOOTH_HALF_WIDTH)), TOOTH_RADIUS);
  const linkAttachY = GEAR_POS_2[1];
  const doorY = f32Sub(linkAttachY, f32Add(f32Mul(f32Mul(2, LINK_COUNT), LINK_HALF_LENGTH), DOOR_HALF_HEIGHT));
  const doorPosition: Vec3 = [linkAttachX, doorY, 0];
  const near = createChain(world, runtime, follower, [linkAttachX, linkAttachY, -GEAR_Z]);
  const far = createChain(world, runtime, follower, [linkAttachX, linkAttachY, GEAR_Z]);
  const door = createDoor(world, runtime, stair.body, doorPosition, near.last, far.last);
  const debris = createDebris(world, runtime);

  return {
    handles: [stair.body, driver, follower, ...near.handles, ...far.handles, door, ...debris],
    driverJoint,
    mesh: stair.mesh,
  };
}

export function buildGearLiftDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  return buildGearLiftScene(world, runtime).handles;
}

export function createGearLiftBodies(runtime: Box3DRuntime): RenderBody[] {
  const wall = wallBox();
  const driverRadius = f32Add(GEAR_RADIUS, TOOTH_HALF_HEIGHT);
  const followerRadius = f32Add(GEAR_RADIUS, TOOTH_HALF_WIDTH);
  const linkAttachX = f32Add(f32Add(f32Add(GEAR_POS_2[0], GEAR_RADIUS), f32Mul(2, TOOTH_HALF_WIDTH)), TOOTH_RADIUS);
  const linkAttachY = GEAR_POS_2[1];
  const doorY = f32Sub(linkAttachY, f32Add(f32Mul(f32Mul(2, LINK_COUNT), LINK_HALF_LENGTH), DOOR_HALF_HEIGHT));
  const bodies: RenderBody[] = [
    {
      kind: "compound",
      type: BodyType.Static,
      position: [0, 0, 0],
      parts: [{
        kind: "box",
        size: [2 * wall.half[0], 2 * wall.half[1], 2 * wall.half[2]],
        position: wall.center,
        color: COLOR_STAIR,
      }],
    },
    { kind: "compound", position: GEAR_POS_1, parts: gearParts(runtime, driverRadius) },
    { kind: "compound", position: GEAR_POS_2, parts: gearParts(runtime, followerRadius) },
  ];
  for (const z of [-GEAR_Z, GEAR_Z]) {
    const y0 = f32Sub(linkAttachY, LINK_HALF_LENGTH);
    for (let i = 0; i < LINK_COUNT; i++) {
      bodies.push({
        kind: "capsule",
        radius: LINK_RADIUS,
        length: f32Mul(2, LINK_HALF_LENGTH),
        axis: "y",
        position: [linkAttachX, f32Sub(y0, f32Mul(i, f32Mul(2, LINK_HALF_LENGTH))), z],
        color: COLOR_LINK,
      });
    }
  }
  bodies.push({
    kind: "box",
    size: [0.1, 2 * DOOR_HALF_HEIGHT, 2 * DOOR_HALF_DEPTH],
    position: [linkAttachX, doorY, 0],
    color: COLOR_DOOR,
  });
  const rockHull = runtime.createRock(ROCK_RADIUS);
  const rockPoints = pointsToTuples(runtime.getHullPoints(rockHull));
  runtime.destroyHull(rockHull);
  forEachDebrisPose(runtime, new Box3DRng(), (position, rotation, color) => {
    bodies.push({ kind: "hull", points: rockPoints, position, rotation, color });
  });
  return bodies;
}

export const dumpSampleName = "Gear Lift";
export const dumpSampleId = "joints/gear-lift";
export const dumpCppSampleName = "Gear Lift";
export const dumpGroundSize = gearLiftGroundSize;
export const dumpBuildDynamicBodies = buildGearLiftDynamicBodies;
