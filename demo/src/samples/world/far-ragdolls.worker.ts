import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFarRagdollsDynamicBodies, buildFarRagdollsGround, farRagdollsGroundSize } from "./far-ragdolls-scene";

class FarRagdollsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return farRagdollsGroundSize();
  }

  protected setupGround(): void {
    buildFarRagdollsGround(this.world!);
  }

  protected async buildScene(): Promise<number[]> {
    return buildFarRagdollsDynamicBodies(this.world!, this.runtime!);
  }
}

new FarRagdollsWorker();
