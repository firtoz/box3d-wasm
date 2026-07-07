import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildOverflowColorPileDynamicBodies, overflowColorPileGroundSize } from "./overflow-color-pile-scene";

class OverflowColorPileWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return overflowColorPileGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildOverflowColorPileDynamicBodies(this.world!, this.runtime!);
  }
}

new OverflowColorPileWorker();
