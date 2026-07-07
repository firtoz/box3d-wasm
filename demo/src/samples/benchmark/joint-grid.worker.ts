import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildJointGridDynamicBodies, jointGridGroundSize } from "./joint-grid-scene";

class JointGridWorker extends PhysicsWorkerBase {
  protected setupGround(): void {}

  protected getGroundSize(): Vec3 { return jointGridGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    this.world!.enableSleeping(false);
    return buildJointGridDynamicBodies(this.world!, this.runtime!);
  }
}

new JointGridWorker();
