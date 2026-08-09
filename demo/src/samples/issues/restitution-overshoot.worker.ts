import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildRestitutionOvershootBodies,
  restitutionOvershootGroundSize,
} from "./restitution-overshoot-scene";

class RestitutionOvershootWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    // Upstream builds a custom small floor; no AddGroundBox.
  }

  protected getGroundSize(): Vec3 {
    return restitutionOvershootGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildRestitutionOvershootBodies(this.world!, this.runtime!);
  }
}

new RestitutionOvershootWorker();
