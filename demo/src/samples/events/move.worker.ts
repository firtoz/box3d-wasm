import {type Vec3, type BodyId} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildMoveEventDynamicBodies, moveEventGroundSize } from "./move-scene";

class MoveEventWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return moveEventGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildMoveEventDynamicBodies(this.world!, this.runtime!);
  }
}

new MoveEventWorker();
