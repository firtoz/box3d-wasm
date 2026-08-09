import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildCylinderDynamicBodies, cylinderGroundSize } from "./cylinder-scene";

class CylinderWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return cylinderGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCylinderDynamicBodies(this.world!, this.runtime!);
  }
}

new CylinderWorker();
