import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildIsotropicFrictionDynamicBodies, isotropicFrictionGroundSize } from "./isotropic-friction-scene";

class IsotropicFrictionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return isotropicFrictionGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildIsotropicFrictionDynamicBodies(this.world!, this.runtime!);
  }
}

new IsotropicFrictionWorker();
