import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildFarStackDynamicBodies, farStackGroundSize } from "./far-stack-scene";

class FarStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return farStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildFarStackDynamicBodies(this.world!, this.runtime!);
  }
}

new FarStackWorker();
