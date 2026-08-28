import { type BodyId, type HumanHandle, type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { ragdollBoxGroundSize, spawnRagdollBox } from "./box-scene";

class RagdollBoxWorker extends PhysicsWorkerBase {
  private human: HumanHandle | null = null;
  private frictionTorque = 5;
  private hertz = 1;
  private dampingRatio = 0.7;

  protected getGroundSize(): Vec3 { return ragdollBoxGroundSize(); }

  protected async buildScene(): Promise<BodyId[]> {
    const scene = spawnRagdollBox(this.world!, this.runtime!, {
      frictionTorque: this.frictionTorque,
      hertz: this.hertz,
      dampingRatio: this.dampingRatio,
    });
    this.human = scene.human;
    return scene.handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    const human = this.human;
    if (world === null || human === null) return false;
    if (msg.type === "set-friction" && typeof msg.value === "number") {
      this.frictionTorque = msg.value;
      world.setHumanJointFrictionTorque(human, msg.value);
      return true;
    }
    if (msg.type === "set-hertz" && typeof msg.value === "number") {
      this.hertz = msg.value;
      world.setHumanJointSpringHertz(human, msg.value);
      return true;
    }
    if (msg.type === "set-damping" && typeof msg.value === "number") {
      this.dampingRatio = msg.value;
      world.setHumanJointDampingRatio(human, msg.value);
      return true;
    }
    if (msg.type === "respawn") {
      void this.restartScene();
      return true;
    }
    return false;
  }
}

new RagdollBoxWorker();
