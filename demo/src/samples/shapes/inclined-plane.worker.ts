import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildInclinedPlaneDynamicBodies, inclinedPlaneGroundSize } from "./inclined-plane-scene";

class ShapesInclinedPlaneWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return inclinedPlaneGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildInclinedPlaneDynamicBodies(this.world!, this.runtime!);
  }
}

new ShapesInclinedPlaneWorker();
