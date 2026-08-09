import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildRollingResistanceDynamicBodies, rollingResistanceGroundSize } from "./rolling-resistance-scene";

class RollingResistanceWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return rollingResistanceGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildRollingResistanceDynamicBodies(this.world!, this.runtime!);
  }
}

new RollingResistanceWorker();
