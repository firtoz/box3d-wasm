import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildRagdollBoxDynamicBodies, ragdollBoxGroundSize } from "./box-scene";

class RagdollBoxWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return ragdollBoxGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildRagdollBoxDynamicBodies(this.world!, this.runtime!); }
}

new RagdollBoxWorker();
