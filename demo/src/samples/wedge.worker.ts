import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildWedgeDynamicBodies, wedgeGroundSize } from "./wedge-scene";

class WedgeWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return wedgeGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildWedgeDynamicBodies(this.world!, this.runtime!);
  }
}

new WedgeWorker();
