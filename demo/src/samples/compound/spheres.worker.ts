import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildCompoundSpheresDynamicBodies, compoundSpheresGroundSize } from "./spheres-scene";

class CompoundSpheresWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return compoundSpheresGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildCompoundSpheresDynamicBodies(this.world!, this.runtime!);
  }
}

new CompoundSpheresWorker();
