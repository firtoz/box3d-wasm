import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildCompoundSimpleDynamicBodies, compoundSimpleGroundSize } from "./simple-scene";

class CompoundSimpleWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return compoundSimpleGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCompoundSimpleDynamicBodies(this.world!, this.runtime!);
  }
}

new CompoundSimpleWorker();
