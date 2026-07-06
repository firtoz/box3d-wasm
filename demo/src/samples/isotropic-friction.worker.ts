import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildIsotropicFrictionDynamicBodies, isotropicFrictionGroundSize } from "./isotropic-friction-scene";

class IsotropicFrictionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return isotropicFrictionGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildIsotropicFrictionDynamicBodies(this.world!, this.runtime!);
  }
}

new IsotropicFrictionWorker();
