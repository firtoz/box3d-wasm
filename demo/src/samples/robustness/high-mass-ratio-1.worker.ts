import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildHighMassRatio1DynamicBodies, highMassRatio1GroundSize } from "./high-mass-ratio-1-scene";

class HighMassRatio1Worker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return highMassRatio1GroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildHighMassRatio1DynamicBodies(this.world!, this.runtime!);
  }
}

new HighMassRatio1Worker();
