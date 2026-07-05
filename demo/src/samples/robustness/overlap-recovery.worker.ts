import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class OverlapRecoveryWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const baseCount = 4;
    const extent = 0.5;
    const overlap = 0.25;
    const fraction = 1 - overlap;
    const half: Vec3 = [extent, extent, extent];

    this.world!.setContactTuning(30, 10, 3);

    let bodyIndex = 0;
    let y = extent;

    for (let i = 0; i < baseCount; i++) {
      let x = fraction * extent * (i - baseCount);
      for (let j = i; j < baseCount; j++) {
        const body = this.world!.createBody({
          type: BodyType.Dynamic, position: [x, y, 0],
        });
        this.runtime!.createHullShape(body, half, {});
        handles.push(body);
        bodyIndex++;
        x += 2 * fraction * extent;
      }
      y += 2 * fraction * extent;
    }

    return handles;
  }
}

new OverlapRecoveryWorker();
