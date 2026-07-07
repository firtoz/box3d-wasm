import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildIsFastDynamicBodies, isFastGroundSize } from "./is-fast-scene";

class IsFastWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return isFastGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildIsFastDynamicBodies(this.world!, this.runtime!); }
}

new IsFastWorker();
