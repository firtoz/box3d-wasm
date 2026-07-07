import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildFallingRagdollsDynamicBodies, fallingRagdollsGroundSize } from "./falling-ragdolls-scene";

class FallingRagdollsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return fallingRagdollsGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildFallingRagdollsDynamicBodies(this.world!, this.runtime!); }
}

new FallingRagdollsWorker();
