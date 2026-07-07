import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildConvexJitterDynamicBodies, convexJitterGroundSize } from "./convex-jitter-scene";

class ConvexJitterWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return convexJitterGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildConvexJitterDynamicBodies(this.world!, this.runtime!);
  }
}

new ConvexJitterWorker();
