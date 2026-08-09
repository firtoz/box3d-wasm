import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildHullReductionDynamicBodies, hullReductionGroundSize } from "./hull-reduction-scene";

class HullReductionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullReductionGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildHullReductionDynamicBodies(this.world!, this.runtime!);
  }
}

new HullReductionWorker();
