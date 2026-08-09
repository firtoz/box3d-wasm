import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildCompoundHullsDynamicBodies, compoundHullsGroundSize } from "./hulls-scene";

class CompoundHullsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return compoundHullsGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCompoundHullsDynamicBodies(this.world!, this.runtime!);
  }
}

new CompoundHullsWorker();
