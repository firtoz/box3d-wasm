import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildDoubleDominoDynamicBodies, doubleDominoGroundSize } from "./double-domino-scene";

class DoubleDominoWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return doubleDominoGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildDoubleDominoDynamicBodies(this.world!, this.runtime!);
  }
}

new DoubleDominoWorker();
