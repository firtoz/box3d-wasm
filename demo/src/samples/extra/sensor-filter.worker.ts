import type { BodyId, ShapeId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  EXTRA_SENSOR_FILTER_HUD_INTS,
  buildExtraSensorFilterDynamicBodies,
  extraSensorFilterGroundSize,
  extraSensorFilterHitColor,
  extraSensorFilterRow,
  extraSensorFilterVisitorStart,
} from "./sensor-filter-scene";

class ExtraSensorFilterWorker extends PhysicsWorkerBase {
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Int32Array | null = null;
  private visitor: BodyId | null = null;
  private visitorShape: ShapeId | null = null;
  private overlap = 0;
  private filterOn = true;

  protected getGroundSize(): Vec3 {
    return extraSensorFilterGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { sensorFilterHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.hudBuffer = new SharedArrayBuffer(EXTRA_SENSOR_FILTER_HUD_INTS * 4);
    this.hud = new Int32Array(this.hudBuffer);
    const { handles, visitorShape } = buildExtraSensorFilterDynamicBodies(this.world!, this.runtime!);
    this.visitor = handles[handles.length - 1]!;
    this.visitorShape = visitorShape;
    this.overlap = 0;
    this.filterOn = true;
    return handles;
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    if (this.world === null || this.hud === null || this.visitor === null || this.visitorShape === null) return;
    const visitorShape = this.visitorShape;
    for (const event of this.world.getSensorBeginEvents()) {
      if (event.visitorShapeHandle === visitorShape) this.overlap += 1;
    }
    for (const event of this.world.getSensorEndEvents()) {
      if (event.visitorShapeHandle === visitorShape) this.overlap = Math.max(0, this.overlap - 1);
    }
    this.world.setShapeCustomColor(visitorShape, this.overlap > 0 ? extraSensorFilterHitColor : 0);
    const sensors = this.world.fillSensorEvents();
    this.hud[0] = sensors.beginCount;
    this.hud[1] = sensors.endCount;
    this.hud[2] = this.filterOn ? 1 : 0;
    const transform = this.world.getBodyTransform(this.visitor);
    if (transform.position[1] < -1) {
      this.world.setBodyTransform(this.visitor, extraSensorFilterVisitorStart);
      this.world.setBodyLinearVelocity(this.visitor, [0, 0, 0]);
      this.world.setBodyAngularVelocity(this.visitor, [0, 0, 0]);
      this.overlap = 0;
      this.world.setShapeCustomColor(visitorShape, 0);
    }
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "setFilter") {
      this.filterOn = msg.value === true;
      this.world?.setCustomSensorFilter(extraSensorFilterRow, this.filterOn);
      return true;
    }
    return false;
  }
}

new ExtraSensorFilterWorker();
