import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildOffsetKinematicDynamicBodies, offsetKinematicGroundSize } from "./offset-kinematic-scene";

class OffsetKinematicWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return offsetKinematicGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildOffsetKinematicDynamicBodies(this.world!, this.runtime!);
  }
}

new OffsetKinematicWorker();
