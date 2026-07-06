import { type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildLockMixingDynamicBodies, lockMixingGroundSize } from "./lock-mixing-scene";

class LockMixingWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return lockMixingGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildLockMixingDynamicBodies(this.world!, this.runtime!);
  }
}

new LockMixingWorker();
