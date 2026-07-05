import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class TinyPyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const extent = 0.025;
    const baseCount = 30;
    const half: Vec3 = [extent, extent, extent];

    for (let i = 0; i < baseCount; i++) {
      const y = (2 * i + 1) * extent;
      for (let j = i; j < baseCount; j++) {
        const x = (i + 1) * extent + 2 * (j - i) * extent - baseCount * extent;
        const body = this.world!.createBody({
          type: BodyType.Dynamic, position: [x, y, 0],
        });
        this.runtime!.createHullShape(body, half, {});
        handles.push(body);
      }
    }

    return handles;
  }
}

new TinyPyramidWorker();
