import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildWedgeDynamicBodies, wedgeGroundSize } from "./wedge-scene";

class WedgeWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return wedgeGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildWedgeDynamicBodies(this.world!, this.runtime!);
  }
}

new WedgeWorker();
