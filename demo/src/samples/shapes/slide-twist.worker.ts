import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildSlideTwistDynamicBodies, slideTwistGroundSize } from "./slide-twist-scene";

class SlideTwistWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return slideTwistGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildSlideTwistDynamicBodies(this.world!, this.runtime!); }
}

new SlideTwistWorker();
