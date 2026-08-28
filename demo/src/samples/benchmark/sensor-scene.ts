import { BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type Vec3 } from "box3d-wasm";
import { cameraFromSetView } from "../shared";
import { f32Mul, f32Sub } from "../f32";

export const SENSOR_COLUMN_COUNT = 40;
export const SENSOR_ROW_COUNT = 40;
export const SENSOR_GROUND_COUNT = 81;
export const SENSOR_PASSIVE_COUNT = SENSOR_COLUMN_COUNT * SENSOR_ROW_COUNT;
export const SENSOR_STATIC_COUNT = SENSOR_GROUND_COUNT + SENSOR_PASSIVE_COUNT;
export const SENSOR_VISITOR_CAPACITY = 512;
export const SENSOR_FILTER_ROW = SENSOR_ROW_COUNT >> 1;
const GRID = 3;
const SHIFT = 5;
const Y_START = 10;
const COLOR_LIME = 0x00ff00;
const COLOR_FUCHSIA = 0xff00ff;

function encodeSensorUserData(active: boolean, row: number): number {
  return (active ? 0x40000000 : 0) | (row & 0x3fffffff);
}

export function forEachSensorStatic(callback: (position: [number, number, number], color: number, size: number) => void): void {
  let x = -40 * GRID;
  for (let i = 0; i < SENSOR_GROUND_COUNT; i++) {
    callback([x, 0, 0], 0x505050, 0.96);
    x += GRID;
  }
  const xCenter = 0.5 * SHIFT * SENSOR_COLUMN_COUNT;
  for (let j = 0; j < SENSOR_ROW_COUNT; j++) {
    const y = j * SHIFT + Y_START;
    const color = j === SENSOR_FILTER_ROW ? COLOR_FUCHSIA : 0x94a3b8;
    for (let i = 0; i < SENSOR_COLUMN_COUNT; i++) {
      callback([i * SHIFT - xCenter, y, 0], color, 1);
    }
  }
}

export function buildSensorStatics(world: PhysicsWorld, runtime: Box3DRuntime): BodyId[] {
  const handles: BodyId[] = [];
  const groundHalf = f32Mul(0.48, GRID);
  const groundDef = {
    isSensor: true,
    enableSensorEvents: true,
    userData: encodeSensorUserData(true, 0),
    customColor: 0x505050,
  };
  let x = f32Mul(-40, GRID);
  for (let i = 0; i < SENSOR_GROUND_COUNT; i++) {
    const body = world.createBody({ position: [x, 0, 0] });
    runtime.createHullShape(body, [groundHalf, groundHalf, groundHalf], groundDef);
    handles.push(body);
    x = f32AddGrid(x, GRID);
  }

  const xCenter = f32Mul(0.5, f32Mul(SHIFT, SENSOR_COLUMN_COUNT));
  for (let j = 0; j < SENSOR_ROW_COUNT; j++) {
    const y = f32AddGrid(f32Mul(j, SHIFT), Y_START);
    const filter = j === SENSOR_FILTER_ROW;
    const def = {
      isSensor: true,
      enableSensorEvents: true,
      enableCustomFiltering: filter,
      userData: encodeSensorUserData(false, j),
      customColor: filter ? COLOR_FUCHSIA : 0,
    };
    for (let i = 0; i < SENSOR_COLUMN_COUNT; i++) {
      const px = f32Sub(f32Mul(i, SHIFT), xCenter);
      const body = world.createBody({ position: [px, y, 0] });
      runtime.createHullShape(body, [0.5, 0.5, 0.5], def);
      handles.push(body);
    }
  }
  return handles;
}

function f32AddGrid(a: number, b: number): number {
  return Math.fround(Math.fround(a) + Math.fround(b));
}

export function createSensorRow(world: PhysicsWorld, runtime: Box3DRuntime, y: number): BodyId[] {
  const xCenter = f32Mul(0.5, f32Mul(SHIFT, SENSOR_COLUMN_COUNT));
  const handles: BodyId[] = [];
  for (let i = 0; i < SENSOR_COLUMN_COUNT; i++) {
    const yOffset = runtime.randomFloatRange(-1, 1);
    const body = world.createBody({
      type: BodyType.Dynamic,
      position: [f32Sub(f32Mul(SHIFT, i), xCenter), y + yOffset, 0],
      gravityScale: 0,
      linearVelocity: [0, -5, 0],
    });
    runtime.createSphereShape(body, [0, 0, 0], 0.5, { enableSensorEvents: true });
    handles.push(body);
  }
  return handles;
}

export function processSensorEvents(world: PhysicsWorld, runtime: Box3DRuntime, tracked: BodyId[]): BodyId[] {
  const zombies = new Set<BodyId>();
  for (const event of world.getSensorBeginEvents()) {
    const data = runtime.getShapeUserData(event.sensorShapeHandle);
    const active = ((data >> 30) & 1) !== 0;
    if (active) zombies.add(world.getShapeBody(event.visitorShapeHandle));
    else runtime.setShapeCustomColor(event.visitorShapeHandle, COLOR_LIME);
  }
  for (const event of world.getSensorEndEvents()) {
    try {
      runtime.setShapeCustomColor(event.visitorShapeHandle, 0);
    } catch {
      // visitor may already be destroyed
    }
  }
  if (zombies.size === 0) return tracked;
  const next: BodyId[] = [];
  for (const id of tracked) {
    if (zombies.has(id)) world.destroyBody(id);
    else next.push(id);
  }
  return next;
}

export function dumpCreate(runtime: Box3DRuntime): { world: PhysicsWorld; handles: BodyId[] } {
  const world = runtime.createWorld({ gravity: [0, -10, 0], workerCount: 1 });
  world.setCustomSensorFilter(SENSOR_FILTER_ROW, true);
  return { world, handles: buildSensorStatics(world, runtime) };
}

export function sensorGroundSize(): Vec3 {
  return [40, 1, 40];
}

export const sensorCamera = cameraFromSetView(0, 0, 250, [0, 110, 0]);

export const dumpSampleName = "Sensor";
export const dumpSampleId = "benchmark/sensor";
export const dumpCppSampleName = "Sensor";
export const dumpGroundSize = sensorGroundSize;
