import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildFallingRagdollsDynamicBodies, fallingRagdollsGroundSize } from "./falling-ragdolls-scene";

class FallingRagdollsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return fallingRagdollsGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildFallingRagdollsDynamicBodies(this.world!, this.runtime!); }
}

new FallingRagdollsWorker();
