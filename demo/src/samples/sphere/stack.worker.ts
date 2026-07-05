import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";

class SphereStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [15, 1, 15];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    for (let i = 0, y = 0.75; i < 30; i++, y += 1.5) {
      const body = this.world!.createBody({ type: 2, position: [0, y, 0], isAwake: true });
      this.runtime!.createSphereShape(body, [0, 0, 0], 0.5, { rollingResistance: 0.1 });
      handles.push(body);
    }
    return handles;
  }
}

new SphereStackWorker();
