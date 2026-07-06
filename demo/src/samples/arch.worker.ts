import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { archGroundSize, buildArchDynamicBodies } from "./arch-scene";

class ArchWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return archGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildArchDynamicBodies(this.world!, this.runtime!);
  }
}

new ArchWorker();
