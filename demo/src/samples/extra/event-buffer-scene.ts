import { BodyType, type BodyId, type Box3DRuntime, type PhysicsWorld, type ShapeId, type Vec3 } from "box3d-wasm";
import type { RenderBody, RenderSpec } from "../generic-host";
import { cameraFromSetView } from "../shared";

const VISITOR_HALF: Vec3 = [0.4, 0.4, 0.4];
const SENSOR_HALF: Vec3 = [1.5, 1.5, 1.5];

export function buildExtraEventBufferDynamicBodies(world: PhysicsWorld, runtime: Box3DRuntime): {
  handles: BodyId[];
  visitor: BodyId;
  sensorShape: ShapeId;
} {
  const sensor = world.createBody({ type: BodyType.Static, position: [0, 1.5, 0] });
  const sensorShape = runtime.createHullShape(sensor, SENSOR_HALF, {
    isSensor: true,
    enableSensorEvents: true,
    enableContactEvents: true,
  }).shapeHandle;
  const visitor = world.createBody({ type: BodyType.Dynamic, position: [0, 6, 0] });
  runtime.createHullShape(visitor, VISITOR_HALF, {
    enableSensorEvents: true,
    enableContactEvents: true,
    enableHitEvents: true,
  });
  return { handles: [sensor, visitor], visitor, sensorShape };
}

export function extraEventBufferGroundSize(): Vec3 {
  return [6, 0.5, 6];
}

export const extraEventBufferBodies: RenderBody[] = [
  {
    kind: "box",
    size: [2 * SENSOR_HALF[0], 2 * SENSOR_HALF[1], 2 * SENSOR_HALF[2]],
    position: [0, 1.5, 0],
    type: BodyType.Static,
    color: 0x34d399,
  },
  { kind: "box", size: [0.8, 0.8, 0.8], position: [0, 6, 0], color: 0x60a5fa },
];

export const extraEventBufferCamera: RenderSpec["camera"] = cameraFromSetView(0, 22, 14, [0, 2, 0]);

export const EXTRA_EVENT_BUFFER_HUD_INTS = 10;
