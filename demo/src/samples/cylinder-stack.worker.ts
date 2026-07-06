import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildCylinderStackDynamicBodies, cylinderStackGroundSize } from "./cylinder-stack-scene";

class CylinderStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return cylinderStackGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildCylinderStackDynamicBodies(this.world!, this.runtime!);
  }
}

new CylinderStackWorker();
