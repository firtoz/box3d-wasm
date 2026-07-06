import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildCompoundHullsDynamicBodies, compoundHullsGroundSize } from "./hulls-scene";

class CompoundHullsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return compoundHullsGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildCompoundHullsDynamicBodies(this.world!, this.runtime!);
  }
}

new CompoundHullsWorker();
