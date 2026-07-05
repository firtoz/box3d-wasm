import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Quat, Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

const PI = Math.PI;

class JengaStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [30, 1, 30];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    for (let i = 0; i < 24; i++) {
      const even = (i & 1) === 0;
      const alpha = even ? 0.5 * PI : 0;
      const q: Quat = [0, Math.sin(alpha / 2), 0, Math.cos(alpha / 2)];
      const x = even ? 1.75 : 0;
      const z = even ? 0 : 1.75;
      addBox(this.world!, this.runtime!, handles, [x, 0.5 * i + 0.25, z], [2.5, 0.25, 0.25], q);
      addBox(this.world!, this.runtime!, handles, [-x, 0.5 * i + 0.25, -z], [2.5, 0.25, 0.25], q);
    }
    return handles;
  }
}

new JengaStackWorker();
