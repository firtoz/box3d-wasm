import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildCylinderStackDynamicBodies, cylinderStackGroundSize } from "./cylinder-stack-scene";

class CylinderStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return cylinderStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCylinderStackDynamicBodies(this.world!, this.runtime!);
  }
}

new CylinderStackWorker();
