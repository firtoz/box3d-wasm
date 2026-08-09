import {BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type ShapeId, type Vec3} from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const VISITOR_HALF: Vec3 = [0.5, 0.5, 0.5];
const VISITOR_POSITION: Vec3 = [0, 12.5, 0];
const SENSOR_HALF: Vec3 = [2, 2, 2];
const SENSOR_POSITION: Vec3 = [0, 2, 0];

export interface SensorVisitScene {
  visitorBody: BodyId;
  sensorBody: BodyId;
  visitorShape: ShapeId;
  sensorShape: ShapeId;
}

export interface SensorVisitState {
  visitorBody: BodyId | null;
  visitorShape: ShapeId | null;
  sensorBody: BodyId;
  sensorShape: ShapeId;
}

export function createSensorVisitScene(world: PhysicsWorld, runtime: Box3DRuntime): SensorVisitScene {
  const visitorBody = world.createBody({
    type: BodyType.Dynamic,
    position: VISITOR_POSITION,
  });
  const visitorShape = runtime.createHullShape(visitorBody, VISITOR_HALF, {
    enableSensorEvents: true,
  }).shapeHandle;

  const sensorBody = world.createBody({
    type: BodyType.Kinematic,
    position: SENSOR_POSITION,
  });
  const sensorShape = runtime.createHullShape(sensorBody, SENSOR_HALF, {
    isSensor: true,
    enableSensorEvents: true,
  }).shapeHandle;

  return { visitorBody, sensorBody, visitorShape, sensorShape };
}

export function createSensorVisitState(scene: SensorVisitScene): SensorVisitState {
  return {
    visitorBody: scene.visitorBody,
    visitorShape: scene.visitorShape,
    sensorBody: scene.sensorBody,
    sensorShape: scene.sensorShape,
  };
}

export function buildSensorVisitDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const scene = createSensorVisitScene(world, runtime);
  return [scene.visitorBody, scene.sensorBody];
}

export function sensorVisitGroundSize(): Vec3 {
  return [5, 0.5, 5];
}

export const sensorVisitBodies: RenderBody[] = [
  {
    kind: "box",
    size: [2 * SENSOR_HALF[0], 2 * SENSOR_HALF[1], 2 * SENSOR_HALF[2]],
    position: SENSOR_POSITION,
    type: BodyType.Kinematic,
    color: 0x34d399,
  },
  {
    kind: "box",
    size: [2 * VISITOR_HALF[0], 2 * VISITOR_HALF[1], 2 * VISITOR_HALF[2]],
    position: VISITOR_POSITION,
    color: 0x60a5fa,
  },
];

export const sensorVisitCamera: RenderSpec["camera"] = cameraFromSetView(0, 30, 20, [0, 5, 0]);

export const dumpSampleName = "Sensor Visit";
export const dumpSampleId = "events/sensor-visit";
export const dumpCppSampleName = "Sensor Visit";

export function processSensorVisitPostStep(world: PhysicsWorld, state: SensorVisitState): boolean {
  if (state.visitorBody === null || state.visitorShape === null) return false;

  for (const event of world.getSensorBeginEvents()) {
    if (event.sensorShapeHandle !== state.sensorShape || event.visitorShapeHandle !== state.visitorShape) continue;
    if (world.bodyIsValid(state.visitorBody)) world.destroyBody(state.visitorBody);
    state.visitorBody = null;
    state.visitorShape = null;
    return true;
  }

  return false;
}

export function dumpPostStep(
  world: PhysicsWorld,
  _runtime: Box3DRuntime,
  _handles: readonly BodyId[],
  _frame: number,
  _dt: number,
  state: unknown,
): void {
  processSensorVisitPostStep(world, state as SensorVisitState);
}

export function dumpCreate(runtime: Box3DRuntime): {
  world: PhysicsWorld;
  handles: BodyId[];
  state: SensorVisitState;
} {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  const scene = createSensorVisitScene(world, runtime);
  return {
    world,
    handles: [scene.visitorBody, scene.sensorBody],
    state: createSensorVisitState(scene),
  };
}
