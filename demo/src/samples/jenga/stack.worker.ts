import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildJengaStackDynamicBodies, jengaStackGroundSize } from "./stack-scene";

class JengaStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return jengaStackGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildJengaStackDynamicBodies(this.world!, this.runtime!);
  }
}

new JengaStackWorker();
