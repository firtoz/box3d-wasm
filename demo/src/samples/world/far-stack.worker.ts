import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFarStackDynamicBodies, farStackGroundSize } from "./far-stack-scene";

class FarStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return farStackGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildFarStackDynamicBodies(this.world!, this.runtime!);
  }
}

new FarStackWorker();
