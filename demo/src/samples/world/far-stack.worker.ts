import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class FarStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [12, 1, 12];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const offset = 10000;
    const half: Vec3 = [0.5, 0.5, 0.5];

    for (let i = 0; i < 6; i++) {
      const skew = 0.02 * (i & 1 ? 1 : -1);
      const body = this.world!.createBody({
        type: BodyType.Dynamic,
        position: [offset + skew, 0.5 + 1 * i, 0],
      });
      this.runtime!.createHullShape(body, half, {});
      handles.push(body);
    }

    return handles;
  }
}

new FarStackWorker();
