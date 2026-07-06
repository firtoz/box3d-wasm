import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildCardHouseDynamicBodies, cardHouseGroundSize } from "./card-house-scene";

class CardHouseWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return cardHouseGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildCardHouseDynamicBodies(this.world!, this.runtime!);
  }
}

new CardHouseWorker();
