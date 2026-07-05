import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { addBox } from "./shared-worker";

class Pyramid2dWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [40, 1, 40];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    for (let row = 0; row < 12; row++) {
      for (let col = 0; col < 12 - row; col++) addBox(this.world!, this.runtime!, handles, [(-10 + 2 * col + row), 1.5 + 2.5 * row, 0], [1, 1, 1], [0, 0, 0, 1], { lock2d: true });
    }
    return handles;
  }
}

new Pyramid2dWorker();
