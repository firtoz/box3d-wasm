import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFarPyramidDynamicBodies, farPyramidGroundSize } from "./far-pyramid-scene";

class FarPyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return farPyramidGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildFarPyramidDynamicBodies(this.world!, this.runtime!);
  }
}

new FarPyramidWorker();
