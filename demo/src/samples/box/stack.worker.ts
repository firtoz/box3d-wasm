import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

class BoxStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [40, 1, 40];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    for (let i = 0; i < 40; i++) addBox(this.world!, this.runtime!, handles, [0, 0.75 + 1.25 * i, 0], [0.5, 0.5, 0.5], [0, 0, 0, 1], { rollingResistance: 0.1 });
    return handles;
  }
}

new BoxStackWorker();
