import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3, WorldCapacity } from "box3d-wasm";
import {
  buildLargeWorldFloor,
  LARGE_WORLD_SPHERES,
  largeWorldCapacity,
  largeWorldGroundSize,
  stepLargeWorld,
  type LargeWorldState,
} from "./large-world-scene";

class LargeWorldWorker extends PhysicsWorkerBase {
  private sphereHandles: number[] = [];
  private largeWorldState: LargeWorldState = { spheresDropped: 0 };

  protected setupGround(): void {
    buildLargeWorldFloor(this.world!, this.runtime!);
  }

  protected getGroundSize(): Vec3 {
    return largeWorldGroundSize();
  }

  protected getWorldCapacity(): WorldCapacity {
    return largeWorldCapacity;
  }

  protected getTrackedBodyCapacity(): number {
    return LARGE_WORLD_SPHERES;
  }

  protected async buildScene(): Promise<number[]> {
    this.sphereHandles = [];
    this.largeWorldState = { spheresDropped: 0 };
    return this.sphereHandles;
  }

  protected stepPhysics(): void {
    const before = this.sphereHandles.length;
    stepLargeWorld(this.world!, this.runtime!, this.sphereHandles, this.totalSteps, this.largeWorldState);
    if (this.sphereHandles.length !== before) this.setTrackedBodies(this.sphereHandles);
    super.stepPhysics();
  }

  protected onBeforeDisposeWorld(): void {
    this.sphereHandles = [];
    this.largeWorldState = { spheresDropped: 0 };
  }
}

new LargeWorldWorker();
