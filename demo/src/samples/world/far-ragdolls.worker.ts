import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFarRagdollsDynamicBodies, farRagdollsGroundSize } from "./far-ragdolls-scene";

class FarRagdollsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return farRagdollsGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildFarRagdollsDynamicBodies(this.world!, this.runtime!);
  }
}

new FarRagdollsWorker();
