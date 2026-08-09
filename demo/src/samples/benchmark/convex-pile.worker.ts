import { BodyId, Vec3, WorldCapacity } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildConvexPileDynamicBodies,
  buildConvexPileGround,
  CONVEX_PILE_BODY_COUNT,
  convexPileGroundSize,
  convexPileWorldCapacity,
} from "./convex-pile-scene";

class ConvexPileWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    buildConvexPileGround(this.world!, this.runtime!);
  }

  protected getGroundSize(): Vec3 {
    return convexPileGroundSize();
  }

  protected getWorldCapacity(): WorldCapacity {
    return convexPileWorldCapacity;
  }

  protected getTrackedBodyCapacity(): number {
    return CONVEX_PILE_BODY_COUNT;
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildConvexPileDynamicBodies(this.world!, this.runtime!);
  }
}

new ConvexPileWorker();
