import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildCapsuleStackDynamicBodies, capsuleStackGroundSize } from "./stack-scene";

class CapsuleStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return capsuleStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCapsuleStackDynamicBodies(this.world!, this.runtime!);
  }
}

new CapsuleStackWorker();
