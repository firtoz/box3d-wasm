import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildRestitutionDynamicBodies, restitutionGroundSize } from "./restitution-scene";

class RestitutionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return restitutionGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildRestitutionDynamicBodies(this.world!, this.runtime!);
  }
}

new RestitutionWorker();
