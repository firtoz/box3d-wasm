import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildSlideTwistOffCenterDynamicBodies,
  slideTwistOffCenterGroundSize,
} from "./slide-twist-off-center-scene";

class SlideTwistOffCenterWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return slideTwistOffCenterGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildSlideTwistOffCenterDynamicBodies(this.world!, this.runtime!);
  }
}

new SlideTwistOffCenterWorker();
