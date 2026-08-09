import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildCandyCupsDynamicBodies, candyCupsGroundSize } from "./candy-cups-scene";

class CandyCupsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return candyCupsGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCandyCupsDynamicBodies(this.world!, this.runtime!);
  }
}

new CandyCupsWorker();
