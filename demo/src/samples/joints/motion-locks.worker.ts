import { BodyType } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildMotionLocksDynamicBodies, motionLocksGroundSize } from "./motion-locks-scene";

class MotionLocksWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return motionLocksGroundSize(); }
  protected async buildScene(): Promise<number[]> {
    const ground = this.world!.createBody({ type: BodyType.Static, position: [0, -1, 0] });
    this.runtime!.createHullShape(ground, motionLocksGroundSize(), {});
    return [ground, ...buildMotionLocksDynamicBodies(this.world!, this.runtime!)];
  }
}

new MotionLocksWorker();
