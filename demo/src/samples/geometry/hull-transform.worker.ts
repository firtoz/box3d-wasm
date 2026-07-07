import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildHullTransformDynamicBodies, hullTransformGroundSize } from "./hull-transform-scene";

class HullTransformWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullTransformGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildHullTransformDynamicBodies(this.world!, this.runtime!);
  }
}

new HullTransformWorker();
