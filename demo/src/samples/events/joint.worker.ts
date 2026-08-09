import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { createJointEventScene, dumpPostStep, jointEventGroundSize } from "./joint-scene";

class JointEventWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return jointEventGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return createJointEventScene(this.world!, this.runtime!).bodies;
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null) return;
    this.world.step(this.fixedTimeStep, this.subSteps);
    dumpPostStep(this.world, this.runtime, [], this.totalSteps + 1, this.fixedTimeStep, undefined);
    this.totalSteps += 1;
  }
}

new JointEventWorker();
