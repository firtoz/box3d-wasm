import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildCapsuleMassDynamicBodies, capsuleMassGroundSize } from "./capsule-mass-scene";

class CapsuleMassWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    // Upstream Capsule Mass is a geometry visualization with no world bodies.
  }

  protected getGroundSize(): Vec3 {
    return capsuleMassGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildCapsuleMassDynamicBodies(this.world!, this.runtime!);
  }
}

new CapsuleMassWorker();
