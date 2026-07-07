import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { bounceHouseGroundSize, buildBounceHouseDynamicBodies } from "./bounce-house-scene";

class BounceHouseWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return bounceHouseGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildBounceHouseDynamicBodies(this.world!, this.runtime!); }
}

new BounceHouseWorker();
