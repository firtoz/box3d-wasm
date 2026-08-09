import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 , BodyId} from "box3d-wasm";
import {
  buildRagdollMeshDynamicBodies,
  buildRagdollMeshGround,
  ragdollMeshGroundSize,
} from "./mesh-scene";

class RagdollMeshWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    buildRagdollMeshGround(this.world!, this.runtime!);
  }

  protected getGroundSize(): Vec3 {
    return ragdollMeshGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildRagdollMeshDynamicBodies(this.world!, this.runtime!);
  }
}

new RagdollMeshWorker();
