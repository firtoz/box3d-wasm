import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3, WorldCapacity } from "box3d-wasm";
import { Box3DRng } from "../box3d-rng";
import {
  buildDestructionGround,
  DESTRUCTION_MAX_BODY_COUNT,
  DESTRUCTION_SPAWN_STEP,
  destroyDestructionBodies,
  destructionGroundSize,
  destructionWorldCapacity,
  spawnDestructionBodies,
} from "./destruction-scene";

class DestructionWorker extends PhysicsWorkerBase {
  private handles: number[] = [];
  private rng = new Box3DRng();

  protected setupGround(): void {
    buildDestructionGround(this.world!);
  }

  protected getGroundSize(): Vec3 {
    return destructionGroundSize();
  }

  protected getWorldCapacity(): WorldCapacity {
    return destructionWorldCapacity;
  }

  protected getTrackedBodyCapacity(): number {
    return DESTRUCTION_MAX_BODY_COUNT;
  }

  protected async buildScene(): Promise<number[]> {
    this.rng = new Box3DRng();
    this.handles = spawnDestructionBodies(this.world!, this.runtime!, this.rng);
    return this.handles;
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    if (this.totalSteps % DESTRUCTION_SPAWN_STEP !== 0) return;
    destroyDestructionBodies(this.world!, this.handles);
    this.handles = spawnDestructionBodies(this.world!, this.runtime!, this.rng);
    this.setTrackedBodies(this.handles);
  }
}

new DestructionWorker();
