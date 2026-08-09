import {type Vec3, type BodyId} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildWheelDynamicBodies, wheelGroundSize } from "./wheel-scene";

class WheelWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return wheelGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildWheelDynamicBodies(this.world!, this.runtime!);
  }
}

new WheelWorker();
