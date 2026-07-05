import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type Vec3 } from "box3d-wasm";

class FarRagdollsWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [20, 1, 20];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const offset = 1000;
    const count = 20;

    for (let i = 0; i < count; i++) {
      const x = 0.15 * (i - 0.5 * count) + offset;
      const y = 2 + 0.25 * i;
      const z = 0.15 * (0.5 * count - i);
      const human = this.world!.createHuman(
        [x, y, z],
        { frictionTorque: 10, hertz: 0.5, dampingRatio: 0.7, groupIndex: i },
      );
      handles.push(human);
    }

    return handles;
  }
}

new FarRagdollsWorker();
