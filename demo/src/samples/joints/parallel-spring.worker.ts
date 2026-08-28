import { type BodyId, type JointId, type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { createParallelSpringScene, parallelSpringGroundSize } from "./parallel-spring-scene";

class ParallelSpringWorker extends PhysicsWorkerBase {
  private joint: JointId | null = null;

  protected getGroundSize(): Vec3 {
    return parallelSpringGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const scene = createParallelSpringScene(this.world!, this.runtime!);
    this.joint = scene.joint;
    return scene.handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    const joint = this.joint;
    if (world === null || joint === null) return false;
    if (msg.type === "set-hertz" && typeof msg.value === "number") {
      world.setParallelSpringHertz(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    if (msg.type === "set-damping" && typeof msg.value === "number") {
      world.setParallelSpringDampingRatio(joint, msg.value);
      world.wakeJointBodies(joint);
      return true;
    }
    return false;
  }
}

new ParallelSpringWorker();
