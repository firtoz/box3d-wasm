import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildDoubleDominoDynamicBodies, doubleDominoGroundSize } from "./double-domino-scene";

class DoubleDominoWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return doubleDominoGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildDoubleDominoDynamicBodies(this.world!, this.runtime!);
  }
}

new DoubleDominoWorker();
