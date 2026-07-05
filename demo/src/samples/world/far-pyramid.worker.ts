import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class FarPyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [400, 1, 400];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const offset = 10000;
    const baseCount = 40;
    const h = 0.5;
    const shift = h;
    const half: Vec3 = [h, h, h];

    for (let i = 0; i < baseCount; i++) {
      const y = (2 * i + 1) * shift;
      for (let j = i; j < baseCount; j++) {
        const x = (i + 1) * shift + 2 * (j - i) * shift - h * baseCount + offset;
        const body = this.world!.createBody({
          type: BodyType.Dynamic, position: [x, y, 0],
        });
        this.runtime!.createHullShape(body, half, { density: 100 });
        handles.push(body);
      }
    }

    return handles;
  }
}

new FarPyramidWorker();
