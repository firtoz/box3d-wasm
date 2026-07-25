import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import {
  buildSlideTwistOffCenterDynamicBodies,
  slideTwistOffCenterGroundSize,
} from "./slide-twist-off-center-scene";

class SlideTwistOffCenterWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return slideTwistOffCenterGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildSlideTwistOffCenterDynamicBodies(this.world!, this.runtime!);
  }
}

new SlideTwistOffCenterWorker();
