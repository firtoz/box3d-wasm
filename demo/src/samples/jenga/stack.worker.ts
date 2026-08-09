import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildJengaStackDynamicBodies, jengaStackGroundSize } from "./stack-scene";

class JengaStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return jengaStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildJengaStackDynamicBodies(this.world!, this.runtime!);
  }
}

new JengaStackWorker();
