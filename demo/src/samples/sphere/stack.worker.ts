import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildSphereStackDynamicBodies, sphereStackGroundSize } from "./stack-scene";

class SphereStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return sphereStackGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildSphereStackDynamicBodies(this.world!, this.runtime!);
  }
}

new SphereStackWorker();
