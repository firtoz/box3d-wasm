import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFixedRotationDynamicBodies, fixedRotationGroundSize } from "./fixed-rotation-scene";

class FixedRotationWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return fixedRotationGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildFixedRotationDynamicBodies(this.world!, this.runtime!);
  }
}

new FixedRotationWorker();
