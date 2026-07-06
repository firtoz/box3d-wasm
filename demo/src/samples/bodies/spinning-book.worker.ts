import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildSpinningBookDynamicBodies, spinningBookGroundSize } from "./spinning-book-scene";

class SpinningBookWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return spinningBookGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildSpinningBookDynamicBodies(this.world!, this.runtime!);
  }
}

new SpinningBookWorker();
