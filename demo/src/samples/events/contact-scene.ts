import {
  BodyType,
  ShapeType,
  type BodyId,
  type Box3DRuntime,
  type PhysicsWorld,
  type ShapeId,
  type Vec3,
} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

export const CONTACT_DEBRIS_COUNT = 200;
export const CONTACT_DEFAULT_TORQUE = 30000;
export const CONTACT_RAND_SEED = 12345;
const GROUND_EXTENT = 80;
const PLAYER_COLOR = (5 << 24) | 0xdda0dd;
const GOLD = 0xffd700;
const NULL_BODY = 0n as BodyId;

export interface ContactState {
  player: BodyId;
  walls: BodyId;
  coreShape: ShapeId;
  debris: BodyId[];
  wait: number;
  massExtent: number;
  torque: number;
  throttleX: number;
  throttleY: number;
  forward: Vec3;
  right: Vec3;
}

function massExtent(world: PhysicsWorld, body: BodyId): number {
  const aabb = world.computeBodyAABB(body);
  const ex = 0.5 * (aabb.max[0] - aabb.min[0]);
  const ey = 0.5 * (aabb.max[1] - aabb.min[1]);
  const ez = 0.5 * (aabb.max[2] - aabb.min[2]);
  const mass = world.getBodyMass(body);
  return (mass / 3) * (ex + ey + ez);
}

function createWalls(world: PhysicsWorld, runtime: Box3DRuntime): BodyId {
  const walls = world.createBody({ type: BodyType.Static });
  const extent = GROUND_EXTENT + 0.5;
  const height = 5;
  const thickness = 0.5;
  runtime.createTransformedHullShape(walls, [thickness, height, extent], { position: [extent, height, 0] });
  runtime.createTransformedHullShape(walls, [thickness, height, extent], { position: [-extent, height, 0] });
  runtime.createTransformedHullShape(walls, [extent, height, thickness], { position: [0, height, extent] });
  runtime.createTransformedHullShape(walls, [extent, height, thickness], { position: [0, height, -extent] });
  return walls;
}

function createPlayer(world: PhysicsWorld, runtime: Box3DRuntime): { player: BodyId; coreShape: ShapeId } {
  const player = world.createBody({
    type: BodyType.Dynamic,
    position: [0, 1, 0],
    angularDamping: 0.1,
  });
  const core = runtime.createSphereShape(player, [0, 0, 0], 1, {
    enableContactEvents: true,
    friction: 0.6,
    rollingResistance: 0.1,
    customColor: PLAYER_COLOR,
  });
  return { player, coreShape: core.shapeHandle };
}

export function createContactState(world: PhysicsWorld, runtime: Box3DRuntime): ContactState {
  const walls = createWalls(world, runtime);
  const { player, coreShape } = createPlayer(world, runtime);
  const debris: BodyId[] = [];
  for (let i = 0; i < CONTACT_DEBRIS_COUNT; i++) debris.push(NULL_BODY);
  return {
    player,
    walls,
    coreShape,
    debris,
    wait: 0.5,
    massExtent: massExtent(world, player),
    torque: CONTACT_DEFAULT_TORQUE,
    throttleX: 0,
    throttleY: 0,
    forward: [0, 0, 1],
    right: [1, 0, 0],
  };
}

function spawnDebris(world: PhysicsWorld, runtime: Box3DRuntime, state: ContactState): void {
  let index = -1;
  for (let i = 0; i < CONTACT_DEBRIS_COUNT; i++) {
    if (state.debris[i] === NULL_BODY) {
      index = i;
      break;
    }
  }
  if (index === -1) return;

  const a = GROUND_EXTENT - 2;
  const position = runtime.randomVec3([-a, 1, -a], [a, 6, a]);
  const body = world.createBody({
    type: BodyType.Dynamic,
    position,
    rotation: runtime.randomQuat(),
    linearVelocity: runtime.randomVec3Uniform(-2, 2),
    angularVelocity: runtime.randomVec3Uniform(-1, 1),
  });
  state.debris[index] = body;

  const shapeDef = { restitution: 0.3 };
  if ((index + 1) % 3 === 0) {
    runtime.createSphereShape(body, [0, 0, 0], 0.5, shapeDef);
  } else if ((index + 1) % 2 === 0) {
    runtime.createCapsuleShape(body, [0, -0.25, 0], [0, 0.25, 0], 0.25, shapeDef);
  } else {
    runtime.createHullShape(body, [0.4, 0.6, 0.5], shapeDef);
  }
}

function attachDebris(world: PhysicsWorld, runtime: Box3DRuntime, state: ContactState, index: number): void {
  const debrisId = state.debris[index];
  if (debrisId === NULL_BODY || !world.bodyIsValid(debrisId)) return;

  const relative = runtime.invMulBodyTransforms(state.player, debrisId);
  const shapes = world.getBodyShapes(debrisId);
  const shapeId = shapes[0];
  if (shapeId === undefined) return;

  const shapeDef = {
    enableContactEvents: true,
    customColor: GOLD,
    updateBodyMass: false,
  };

  const type = runtime.getShapeType(shapeId);
  if (type === ShapeType.Sphere) {
    const sphere = runtime.getSphere(shapeId);
    runtime.createSphereShape(state.player, runtime.transformPoint(relative, sphere.center), sphere.radius, shapeDef);
  } else if (type === ShapeType.Capsule) {
    const capsule = runtime.getCapsule(shapeId);
    runtime.createCapsuleShape(
      state.player,
      runtime.transformPoint(relative, capsule.center1),
      runtime.transformPoint(relative, capsule.center2),
      capsule.radius,
      shapeDef,
    );
  } else if (type === ShapeType.Hull) {
    const hull = runtime.cloneHullFromShape(shapeId);
    runtime.createTransformedShapeFromHull(state.player, hull, { position: relative.position, rotation: relative.rotation }, [1, 1, 1], shapeDef);
    runtime.destroyHull(hull);
  }

  world.destroyBody(debrisId);
  state.debris[index] = NULL_BODY;
}

export function applyContactDrive(world: PhysicsWorld, runtime: Box3DRuntime, state: ContactState): void {
  const dx = state.throttleX * state.forward[0] + state.throttleY * state.right[0];
  const dy = state.throttleX * state.forward[1] + state.throttleY * state.right[1];
  const dz = state.throttleX * state.forward[2] + state.throttleY * state.right[2];
  const { length, direction } = runtime.getLengthAndNormalize([dx, dy, dz]);
  if (length <= 0) return;
  const scale = massExtent(world, state.player) / state.massExtent;
  const axis: Vec3 = [
    1 * direction[2] - 0 * direction[1],
    0 * direction[0] - 0 * direction[2],
    0 * direction[1] - 1 * direction[0],
  ];
  const torque = scale * state.torque;
  world.applyTorque(state.player, [torque * axis[0], torque * axis[1], torque * axis[2]], true);
}

export function processContactPostStep(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  state: ContactState,
  dt: number,
): void {
  const toAttach: number[] = [];
  const toDestroy: ShapeId[] = [];
  for (const event of world.getContactBeginEvents()) {
    let playerShape = event.shapeA;
    let otherShape = event.shapeB;
    if (world.getShapeBody(playerShape) !== state.player) {
      playerShape = event.shapeB;
      otherShape = event.shapeA;
    }
    if (world.getShapeBody(playerShape) !== state.player) continue;

    const otherBody = world.getShapeBody(otherShape);
    let debrisIndex = -1;
    for (let i = 0; i < CONTACT_DEBRIS_COUNT; i++) {
      if (state.debris[i] === otherBody) {
        debrisIndex = i;
        break;
      }
    }
    if (debrisIndex !== -1) {
      if (toAttach.length < CONTACT_DEBRIS_COUNT) toAttach.push(debrisIndex);
    } else if (otherBody === state.walls && playerShape !== state.coreShape) {
      if (!toDestroy.includes(playerShape) && toDestroy.length < CONTACT_DEBRIS_COUNT) {
        toDestroy.push(playerShape);
      }
    }
  }

  for (const index of toAttach) attachDebris(world, runtime, state, index);
  for (const shape of toDestroy) world.destroyShape(shape, false);
  if (toAttach.length > 0 || toDestroy.length > 0) {
    world.applyBodyMassFromShapes(state.player);
  }

  state.wait -= dt;
  if (state.wait < 0) {
    spawnDebris(world, runtime, state);
    state.wait += 0.5;
  }
}

export function contactTrackedBodies(state: ContactState): BodyId[] {
  const handles: BodyId[] = [state.walls, state.player];
  for (const id of state.debris) handles.push(id);
  return handles;
}

export function syncDumpHandles(world: PhysicsWorld, handles: BodyId[]): void {
  const bodies = world.getWorldBodies();
  handles.length = 0;
  for (const body of bodies) handles.push(body);
}

export function createContactDump(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: ContactState;
} {
  runtime.setRandomSeed(CONTACT_RAND_SEED);
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const ground = world.createBody({ type: BodyType.Static, position: [0, -1, 0] });
  runtime.createHullShape(ground, contactGroundSize());
  const state = createContactState(world, runtime);
  const handles: BodyId[] = [ground, state.walls, state.player];
  return { world, handles, state };
}

export function contactDumpStep(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  _handles: readonly BodyId[],
  _frame: number,
  _dt: number,
  state: unknown,
): void {
  applyContactDrive(world, runtime, state as ContactState);
}

export function contactDumpPostStep(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
  handles: readonly BodyId[],
  _frame: number,
  dt: number,
  state: unknown,
): void {
  processContactPostStep(world, runtime, state as ContactState, dt);
  syncDumpHandles(world, handles as BodyId[]);
}

export function contactGroundSize(): Vec3 {
  return [GROUND_EXTENT, 1, GROUND_EXTENT];
}

function debrisRenderBody(index: number): RenderBody {
  const position: Vec3 = [0, -1000, 0];
  if ((index + 1) % 3 === 0) {
    return { kind: "sphere", radius: 0.5, position, color: 0x60a5fa };
  }
  if ((index + 1) % 2 === 0) {
    return { kind: "capsule", radius: 0.25, length: 0.5, axis: "y", position, color: 0x60a5fa };
  }
  return { kind: "box", size: [0.8, 1.2, 1], position, color: 0x60a5fa };
}

export function createContactBodies(): RenderBody[] {
  const extent = GROUND_EXTENT + 0.5;
  const height = 5;
  const thickness = 0.5;
  const walls: RenderBody = {
    kind: "compound",
    position: [0, 0, 0],
    type: BodyType.Static,
    parts: [
      { kind: "box", size: [2 * thickness, 2 * height, 2 * extent], position: [extent, height, 0], color: 0x64748b },
      { kind: "box", size: [2 * thickness, 2 * height, 2 * extent], position: [-extent, height, 0], color: 0x64748b },
      { kind: "box", size: [2 * extent, 2 * height, 2 * thickness], position: [0, height, extent], color: 0x64748b },
      { kind: "box", size: [2 * extent, 2 * height, 2 * thickness], position: [0, height, -extent], color: 0x64748b },
    ],
  };
  const player: RenderBody = {
    kind: "sphere",
    radius: 1,
    position: [0, 1, 0],
    color: 0xdda0dd,
  };
  const debris: RenderBody[] = [];
  for (let i = 0; i < CONTACT_DEBRIS_COUNT; i++) debris.push(debrisRenderBody(i));
  return [walls, player, ...debris];
}

export const contactCamera: RenderSpec["camera"] = cameraFromSetView(0, 35, 60, [0, 0, 0]);

export const dumpSampleName = "Events / Contact";
export const dumpSampleId = "events/contact";
export const dumpCppSampleName = "Contact";
export const dumpCreate = createContactDump;
export const dumpStep = contactDumpStep;
export const dumpPostStep = contactDumpPostStep;
