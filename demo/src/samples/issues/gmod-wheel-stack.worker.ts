import {type Vec3, type BodyId} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildGmodWheelStackDynamicBodies, gmodWheelStackGroundSize } from "./gmod-wheel-stack-scene";

class GmodWheelStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return gmodWheelStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildGmodWheelStackDynamicBodies(this.world!, this.runtime!);
  }
}

new GmodWheelStackWorker();
