import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildPyramid2dDynamicBodies, pyramid2dGroundSize } from "./pyramid2d-scene";

class Pyramid2dWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return pyramid2dGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildPyramid2dDynamicBodies(this.world!, this.runtime!);
  }
}

new Pyramid2dWorker();
