import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildOverlapRecoveryDynamicBodies, overlapRecoveryGroundSize } from "./overlap-recovery-scene";

class OverlapRecoveryWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return overlapRecoveryGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildOverlapRecoveryDynamicBodies(this.world!, this.runtime!);
  }
}

new OverlapRecoveryWorker();
