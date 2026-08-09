import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { boxStackGroundSize, buildBoxStackDynamicBodies } from "./stack-scene";

class BoxStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return boxStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildBoxStackDynamicBodies(this.world!, this.runtime!);
  }
}

new BoxStackWorker();
