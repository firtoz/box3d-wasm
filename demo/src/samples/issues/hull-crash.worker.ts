import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildHullCrashDynamicBodies, hullCrashGroundSize } from "./hull-crash-scene";

class HullCrashWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return hullCrashGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildHullCrashDynamicBodies(this.world!, this.runtime!);
  }
}

new HullCrashWorker();
