import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { HumanHandle, Vec3 } from "box3d-wasm";
import { buildRagdollInclineScene, ragdollInclineGroundSize } from "./incline-scene";

class RagdollInclineWorker extends PhysicsWorkerBase {
  private human: HumanHandle | null = null;
  private time = 0;
  private motorized = true;

  protected getGroundSize(): Vec3 { return ragdollInclineGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    const { handles, human } = buildRagdollInclineScene(this.world!, this.runtime!);
    this.human = human;
    return handles;
  }

  protected stepPhysics(): void {
    if (this.human !== null && this.time > 2 && this.motorized) {
      this.runtime!.setHumanJointFrictionTorque(this.human, 0.5);
      this.runtime!.setHumanJointSpringHertz(this.human, 0.5);
      this.motorized = false;
    }
    this.time += this.fixedTimeStep;
    super.stepPhysics();
  }
}

new RagdollInclineWorker();
