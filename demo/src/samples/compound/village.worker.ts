import type { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildVillageDynamicBodies, villageGroundSize } from "./village-scene";

class VillageWorker extends PhysicsWorkerBase {
  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return villageGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildVillageDynamicBodies(this.world!, this.runtime!);
  }
}

new VillageWorker();
