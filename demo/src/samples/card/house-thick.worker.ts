import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildCardHouseThickDynamicBodies, cardHouseThickGroundSize } from "./house-thick-scene";

class CardHouseThickWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return cardHouseThickGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildCardHouseThickDynamicBodies(this.world!, this.runtime!);
  }
}

new CardHouseThickWorker();
