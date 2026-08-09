import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 , BodyId} from "box3d-wasm";
import { buildBodyCastDynamicBodies, bodyCastGroundSize } from "./cast-scene";

class BodyCastWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    // Upstream has no physics ground — kinematic cylinder only.
  }

  protected getGroundSize(): Vec3 {
    return bodyCastGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildBodyCastDynamicBodies(this.world!, this.runtime!);
  }
}

new BodyCastWorker();
