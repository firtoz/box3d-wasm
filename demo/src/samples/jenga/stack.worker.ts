import { PhysicsWorkerBase } from "../../physics-worker-base";
import { B3_AXIS_Y, B3_PI, type Vec3 } from "box3d-wasm";
import { addBox } from "../shared-worker";

class JengaStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [30, 1, 30];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    for (let i = 0; i < 24; i++) {
      const even = (i & 1) === 0;
      const alpha = even ? 0.5 * B3_PI : 0;
      const q = this.runtime!.makeQuatFromAxisAngle(B3_AXIS_Y, alpha);
      const x = even ? 1.75 : 0;
      const z = even ? 0 : 1.75;
      addBox(this.world!, this.runtime!, handles, [x, 0.5 * i + 0.25, z], [2.5, 0.25, 0.25], q);
      addBox(this.world!, this.runtime!, handles, [-x, 0.5 * i + 0.25, -z], [2.5, 0.25, 0.25], q);
    }
    return handles;
  }
}

new JengaStackWorker();
