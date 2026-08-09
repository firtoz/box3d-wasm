import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildSlideTwistDynamicBodies, slideTwistGroundSize } from "./slide-twist-scene";

class SlideTwistWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return slideTwistGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildSlideTwistDynamicBodies(this.world!, this.runtime!); }
}

new SlideTwistWorker();
