import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { archGroundSize, buildArchDynamicBodies } from "./arch-scene";

class ArchWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return archGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildArchDynamicBodies(this.world!, this.runtime!);
  }
}

new ArchWorker();
