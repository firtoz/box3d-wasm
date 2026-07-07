import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildRagdollBoxDynamicBodies, ragdollBoxGroundSize } from "./box-scene";

class RagdollBoxWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return ragdollBoxGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildRagdollBoxDynamicBodies(this.world!, this.runtime!); }
}

new RagdollBoxWorker();
