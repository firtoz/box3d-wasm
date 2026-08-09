import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildFixedRotationDynamicBodies, fixedRotationGroundSize } from "./fixed-rotation-scene";

class FixedRotationWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return fixedRotationGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildFixedRotationDynamicBodies(this.world!, this.runtime!);
  }
}

new FixedRotationWorker();
