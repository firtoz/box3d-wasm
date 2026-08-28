import { B3W_SENSOR_FILTER_ACTIVE_BIT, BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type ShapeId, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const SENSOR_HALF: Vec3 = [0.6, 0.6, 0.6];
const VISITOR_HALF: Vec3 = [0.3, 0.3, 0.3];
const FILTER_ROW = 1;
const SENSOR_Y: readonly number[] = [4.5, 2.5, 0.5];
const VISITOR_START: Vec3 = [0, 8, 0];
const VISITOR_HIT_COLOR = 0x00ff00;

function encodeSensorUserData(active: boolean, row: number): number {
  return (active ? B3W_SENSOR_FILTER_ACTIVE_BIT : 0) | (row & 0x3fffffff);
}

export function buildExtraSensorFilterDynamicBodies(
  world: PhysicsWorld,
  runtime: Box3DRuntime,
): { handles: BodyId[]; visitorShape: ShapeId } {
  world.setCustomSensorFilter(FILTER_ROW, true);
  const handles: BodyId[] = [];
  for (let row = 0; row < 3; row++) {
    const body = world.createBody({ type: BodyType.Static, position: [0, SENSOR_Y[row]!, 0] });
    runtime.createHullShape(body, SENSOR_HALF, {
      isSensor: true,
      enableSensorEvents: true,
      enableCustomFiltering: row === FILTER_ROW,
      userData: encodeSensorUserData(false, row),
      customColor: row === FILTER_ROW ? 0xff00ff : 0x94a3b8,
    });
    handles.push(body);
  }
  const visitor = world.createBody({ type: BodyType.Dynamic, position: VISITOR_START });
  const visitorShape = runtime.createHullShape(visitor, VISITOR_HALF, { enableSensorEvents: true }).shapeHandle;
  handles.push(visitor);
  return { handles, visitorShape };
}

export function extraSensorFilterGroundSize(): Vec3 {
  return [8, 0.5, 8];
}

export const extraSensorFilterBodies: RenderBody[] = [
  { kind: "box", size: [1.2, 1.2, 1.2], position: [0, SENSOR_Y[0]!, 0], type: BodyType.Static, color: 0x94a3b8 },
  { kind: "box", size: [1.2, 1.2, 1.2], position: [0, SENSOR_Y[1]!, 0], type: BodyType.Static, color: 0xff00ff },
  { kind: "box", size: [1.2, 1.2, 1.2], position: [0, SENSOR_Y[2]!, 0], type: BodyType.Static, color: 0x94a3b8 },
  { kind: "box", size: [0.6, 0.6, 0.6], position: [...VISITOR_START], color: 0x60a5fa },
];

export const extraSensorFilterCamera: RenderSpec["camera"] = cameraFromSetView(0, 22, 16, [0, 2.5, 0]);

export const EXTRA_SENSOR_FILTER_HUD_INTS = 3;
export const extraSensorFilterRow = FILTER_ROW;
export const extraSensorFilterVisitorStart = VISITOR_START;
export const extraSensorFilterHitColor = VISITOR_HIT_COLOR;
