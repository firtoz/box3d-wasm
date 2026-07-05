import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

class SingleBoxWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [20, 1, 20];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    addBox(this.world!, this.runtime!, handles, [0, 0.5, 0], [0.5, 0.5, 0.5], [0, 0, 0, 1], { angularVelocity: [0, 10, 0] });
    return handles;
  }
}

new SingleBoxWorker();
