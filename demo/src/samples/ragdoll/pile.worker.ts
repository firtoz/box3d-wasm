import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildRagdollPileDynamicBodies, buildRagdollPileGround, ragdollPileGroundSize } from "./pile-scene";

class RagdollPileWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    buildRagdollPileGround(this.world!);
  }

  protected getGroundSize(): Vec3 { return ragdollPileGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    return buildRagdollPileDynamicBodies(this.world!, this.runtime!);
  }
}

new RagdollPileWorker();
