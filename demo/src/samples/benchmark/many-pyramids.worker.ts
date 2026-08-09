import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildManyPyramidsDynamicBodies, manyPyramidsGroundSize } from "./many-pyramids-scene";

class ManyPyramidsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return manyPyramidsGroundSize(); }

  protected async buildScene(): Promise<BodyId[]> {
    this.world!.enableSleeping(false);
    return buildManyPyramidsDynamicBodies(this.world!, this.runtime!);
  }
}

new ManyPyramidsWorker();
