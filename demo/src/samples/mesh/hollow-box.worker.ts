import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 , BodyId} from "box3d-wasm";
import { buildHollowBoxDynamicBodies, buildHollowBoxGround, hollowBoxGroundSize } from "./hollow-box-scene";

class HollowBoxWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    buildHollowBoxGround(this.world!);
  }

  protected getGroundSize(): Vec3 {
    return hollowBoxGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildHollowBoxDynamicBodies(this.world!, this.runtime!);
  }
}

new HollowBoxWorker();
