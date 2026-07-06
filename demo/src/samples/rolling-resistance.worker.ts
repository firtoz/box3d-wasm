import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildRollingResistanceDynamicBodies, rollingResistanceGroundSize } from "./rolling-resistance-scene";

class RollingResistanceWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return rollingResistanceGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildRollingResistanceDynamicBodies(this.world!, this.runtime!);
  }
}

new RollingResistanceWorker();
