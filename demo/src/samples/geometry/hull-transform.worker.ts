import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildHullTransformDynamicBodies, hullTransformGroundSize } from "./hull-transform-scene";

class HullTransformWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullTransformGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildHullTransformDynamicBodies(this.world!, this.runtime!);
  }
}

new HullTransformWorker();
