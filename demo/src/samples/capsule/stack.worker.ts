import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";

class CapsuleStackWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [20, 1, 20];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    for (let i = 0, y = 0.75; i < 20; i++, y += 1) {
      const body = this.world!.createBody({ type: 2, position: [0, y, 0], isAwake: true });
      this.runtime!.createCapsuleShape(body, [-1, 0, 0], [1, 0, 0], 0.5);
      this.runtime!.setBodyMotionLocks(body, { lockLinearZ: true, lockRotationX: true, lockRotationY: true, lockRotationZ: true });
      handles.push(body);
    }
    return handles;
  }
}

new CapsuleStackWorker();
