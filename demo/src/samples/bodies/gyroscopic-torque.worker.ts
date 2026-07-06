import { type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildGyroscopicTorqueDynamicBodies, gyroscopicTorqueGroundSize } from "./gyroscopic-torque-scene";

class GyroscopicTorqueWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return gyroscopicTorqueGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildGyroscopicTorqueDynamicBodies(this.world!, this.runtime!);
  }
}

new GyroscopicTorqueWorker();
