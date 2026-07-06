import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { boxStackGroundSize, buildBoxStackDynamicBodies } from "./stack-scene";

class BoxStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return boxStackGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildBoxStackDynamicBodies(this.world!, this.runtime!);
  }
}

new BoxStackWorker();
