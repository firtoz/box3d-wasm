import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildHullDynamicBodies, hullGroundSize } from "./hull-scene";

class HullWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildHullDynamicBodies(this.world!, this.runtime!);
  }
}

new HullWorker();
