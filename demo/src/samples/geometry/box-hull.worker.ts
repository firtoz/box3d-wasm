import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildBoxHullDynamicBodies, boxHullGroundSize } from "./box-hull-scene";

class BoxHullWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return boxHullGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildBoxHullDynamicBodies(this.world!, this.runtime!);
  }
}

new BoxHullWorker();
