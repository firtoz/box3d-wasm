import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class HighMassRatio1Worker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [25, 1, 25];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const extent = 1;
    const half: Vec3 = [extent, extent, extent];

    for (let j = 0; j < 3; j++) {
      let count = 10;
      const offset = -20 * extent + 2 * (count + 1) * extent * j;
      let y = extent;
      while (count > 0) {
        for (let i = 0; i < count; i++) {
          const coeff = i - 0.5 * count;
          const yy = count === 1 ? y + 2 : y;
          const density = count === 1 ? (j + 1) * 100 : 1;

          const body = this.world!.createBody({
            type: BodyType.Dynamic,
            position: [2 * coeff * extent + offset, yy, 0],
          });
          this.runtime!.createHullShape(body, half, { density });
          handles.push(body);
        }
        count--;
        y += 2 * extent;
      }
    }

    return handles;
  }
}

new HighMassRatio1Worker();
