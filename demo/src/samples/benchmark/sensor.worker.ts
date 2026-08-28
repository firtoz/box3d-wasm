import type { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  SENSOR_STATIC_COUNT,
  SENSOR_VISITOR_CAPACITY,
  SENSOR_FILTER_ROW,
  SENSOR_ROW_COUNT,
  buildSensorStatics,
  createSensorRow,
  processSensorEvents,
  sensorGroundSize,
} from "./sensor-scene";

class SensorWorker extends PhysicsWorkerBase {
  private statics: BodyId[] = [];
  private visitors: BodyId[] = [];

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return sensorGroundSize();
  }

  protected getTrackedBodyCapacity(): number {
    return SENSOR_STATIC_COUNT + SENSOR_VISITOR_CAPACITY;
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.runtime!.setRandomSeed(42);
    this.world!.setCustomSensorFilter(SENSOR_FILTER_ROW, true);
    this.statics = buildSensorStatics(this.world!, this.runtime!);
    this.visitors = [];
    return [...this.statics];
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null) return;
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
    this.visitors = processSensorEvents(this.world, this.runtime, this.visitors);
    if ((this.totalSteps & 0x1f) === 0) {
      this.visitors.push(...createSensorRow(this.world, this.runtime, 10 + SENSOR_ROW_COUNT * 5));
      if (this.visitors.length > SENSOR_VISITOR_CAPACITY) {
        const extra = this.visitors.splice(0, this.visitors.length - SENSOR_VISITOR_CAPACITY);
        for (const id of extra) this.world.destroyBody(id);
      }
    }
    this.setTrackedBodies([...this.statics, ...this.visitors]);
  }
}

new SensorWorker();
