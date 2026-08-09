import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildTinyPyramidDynamicBodies, tinyPyramidGroundSize } from "./tiny-pyramid-scene";

class TinyPyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return tinyPyramidGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildTinyPyramidDynamicBodies(this.world!, this.runtime!);
  }
}

new TinyPyramidWorker();
