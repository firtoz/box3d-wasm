import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildRestitutionDynamicBodies, restitutionGroundSize } from "./restitution-scene";

class RestitutionWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return restitutionGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildRestitutionDynamicBodies(this.world!, this.runtime!);
  }
}

new RestitutionWorker();
