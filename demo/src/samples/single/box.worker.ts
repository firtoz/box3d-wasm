import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildSingleBoxDynamicBodies, singleBoxGroundSize } from "./box-scene";

class SingleBoxWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return singleBoxGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildSingleBoxDynamicBodies(this.world!, this.runtime!);
  }
}

new SingleBoxWorker();
