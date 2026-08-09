import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildBoxHullDynamicBodies, boxHullGroundSize } from "./box-hull-scene";

class BoxHullWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return boxHullGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildBoxHullDynamicBodies(this.world!, this.runtime!);
  }
}

new BoxHullWorker();
