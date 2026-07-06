import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildCardHouseThickDynamicBodies, cardHouseThickGroundSize } from "./house-thick-scene";

class CardHouseThickWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return cardHouseThickGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildCardHouseThickDynamicBodies(this.world!, this.runtime!);
  }
}

new CardHouseThickWorker();
