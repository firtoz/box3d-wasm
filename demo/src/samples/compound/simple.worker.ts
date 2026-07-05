import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Quat, Vec3 } from "box3d-wasm";

const PI = Math.PI;

class CompoundSimpleWorker extends PhysicsWorkerBase {
  private compound = 0;

  protected getGroundSize(): Vec3 {
    return [20, 1, 20];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];

    // Clean up previous compound on restart
    if (this.compound !== 0) {
      this.runtime!.destroyCompound(this.compound);
    }

    // Create compound with one hull matching C++ SimpleCompound
    this.compound = this.runtime!.createCompoundFromHulls([
      {
        halfWidths: [4, 0.5, 4],
        transform: { position: [1, -0.5, 0], rotation: [0, 0, 0, 1] },
        friction: 0.5,
      },
    ]);

    // Static body with compound shape
    const q: Quat = [0, Math.sin(PI / 8), 0, Math.cos(PI / 8)];
    const body = this.world!.createBody({ type: 0, position: [2, -1, 0], rotation: q });
    this.runtime!.createCompoundShape(body, this.compound);
    handles.push(body);

    // Dynamic sphere
    const sphere = this.world!.createBody({ type: 2, position: [0, 2, 0], isAwake: true });
    this.runtime!.createSphereShape(sphere, [0, 0, 0], 0.25);
    handles.push(sphere);

    return handles;
  }
}

new CompoundSimpleWorker();
