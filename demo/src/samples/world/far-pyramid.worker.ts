import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFarPyramidDynamicBodies, buildFarPyramidGround, farPyramidGroundSize } from "./far-pyramid-scene";

class FarPyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return farPyramidGroundSize();
  }

  protected setupGround(): void {
    buildFarPyramidGround(this.world!, this.runtime!);
  }

  protected async buildScene(): Promise<number[]> {
    return buildFarPyramidDynamicBodies(this.world!, this.runtime!);
  }
}

new FarPyramidWorker();
