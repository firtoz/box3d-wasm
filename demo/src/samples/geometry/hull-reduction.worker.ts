import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildHullReductionDynamicBodies, hullReductionGroundSize } from "./hull-reduction-scene";

class HullReductionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullReductionGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildHullReductionDynamicBodies(this.world!, this.runtime!);
  }
}

new HullReductionWorker();
