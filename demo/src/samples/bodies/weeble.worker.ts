import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";

class WeebleWorker extends PhysicsWorkerBase {
  private weebleId = 0;

  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const W = this.world!;
    const R = this.runtime!;

    this.weebleId = W.createBody({
      type: 1,
      position: [0, 10, 0],
    });
    R.createCapsuleShape(this.weebleId, [0, -0.5, 0], [0, 0.5, 0], 0.25, {
      density: 1,
      rollingResistance: 0.1,
    });

    const massData = R.getBodyMassData(this.weebleId);
    const inertia = R.getBodyLocalRotationalInertia(this.weebleId);
    R.setBodyMassData(this.weebleId, massData.mass, [0, 0.26, 0], inertia);

    return [this.weebleId];
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const R = this.runtime!;
    const W = this.world!;
    switch (msg.type) {
      case "teleport": {
        R.setBodyTransform(this.weebleId, [0, 20, 0], [0, 0, 0, 1]);
        R.setBodyAwake(this.weebleId, true);
        return true;
      }
      case "explode": {
        W.explode([0, 0, 0], 30, 2, 10);
        return true;
      }
    }
    return false;
  }
}

new WeebleWorker();
