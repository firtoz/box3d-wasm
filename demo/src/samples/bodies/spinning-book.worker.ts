import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildSpinningBookDynamicBodies, spinningBookGroundSize } from "./spinning-book-scene";

class SpinningBookWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return spinningBookGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildSpinningBookDynamicBodies(this.world!, this.runtime!);
  }
}

new SpinningBookWorker();
