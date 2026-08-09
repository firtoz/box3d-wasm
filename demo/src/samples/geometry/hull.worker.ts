import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildHullDynamicBodies, hullGroundSize } from "./hull-scene";

class HullWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildHullDynamicBodies(this.world!, this.runtime!);
  }
}

new HullWorker();
