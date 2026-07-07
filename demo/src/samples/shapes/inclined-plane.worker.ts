import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildInclinedPlaneDynamicBodies, inclinedPlaneGroundSize } from "./inclined-plane-scene";

class ShapesInclinedPlaneWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return inclinedPlaneGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildInclinedPlaneDynamicBodies(this.world!, this.runtime!);
  }
}

new ShapesInclinedPlaneWorker();
