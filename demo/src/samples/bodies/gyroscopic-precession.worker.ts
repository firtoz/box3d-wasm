import {B3_AXIS_Y, B3_PI, type BodyId, type Vec3} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildGyroscopicPrecessionDynamicBodies,
  gyroscopicPrecessionGroundSize,
} from "./gyroscopic-precession-scene";

/** debug SAB: [awake, spin, tiltDeg] */
const DEBUG_FLOATS = 3;

class GyroscopicPrecessionWorker extends PhysicsWorkerBase {
  private topId: BodyId | null = null;
  private debugBuffer: SharedArrayBuffer | null = null;
  private debugValues: Float32Array | null = null;

  protected getGroundSize(): Vec3 {
    return gyroscopicPrecessionGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const handles = buildGyroscopicPrecessionDynamicBodies(this.world!, this.runtime!);
    this.topId = handles[0] ?? null;
    this.debugBuffer = new SharedArrayBuffer(DEBUG_FLOATS * 4);
    this.debugValues = new Float32Array(this.debugBuffer);
    return handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.debugBuffer === null ? {} : { debug: this.debugBuffer };
  }

  protected publishExtra(): void {
    if (this.world === null || this.runtime === null || this.topId === null || this.debugValues === null) return;
    const awake = this.world.bodyIsAwake(this.topId);
    if (!awake) {
      this.debugValues[0] = 0;
      return;
    }
    this.debugValues[0] = 1;
    const rotation = this.world.getBodyTransform(this.topId).rotation;
    const axis = this.runtime.rotateVector(rotation, B3_AXIS_Y);
    const omega = this.world.getBodyAngularVelocity(this.topId);
    const spin = omega[0] * axis[0] + omega[1] * axis[1] + omega[2] * axis[2];
    const cosTilt = Math.min(1, Math.max(-1, axis[1]));
    this.debugValues[1] = spin;
    this.debugValues[2] = (180 / B3_PI) * Math.acos(cosTilt);
  }
}

new GyroscopicPrecessionWorker();
