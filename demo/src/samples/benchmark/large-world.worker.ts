import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3, WorldCapacity } from "box3d-wasm";
import {
  buildLargeWorldFloor,
  LARGE_WORLD_GRID,
  LARGE_WORLD_SPHERES,
  largeWorldCapacity,
  largeWorldGroundSize,
  largeWorldLiveScale,
  stepLargeWorld,
  type LargeWorldState,
} from "./large-world-scene";

class LargeWorldWorker extends PhysicsWorkerBase {
  private sphereHandles: number[] = [];
  private largeWorldState: LargeWorldState = { spheresDropped: 0, scale: largeWorldLiveScale };

  protected setupGround(): void {
    buildLargeWorldFloor(this.world!, this.runtime!, LARGE_WORLD_GRID);
  }

  protected getGroundSize(): Vec3 {
    return largeWorldGroundSize(LARGE_WORLD_GRID);
  }

  protected getWorldCapacity(): WorldCapacity {
    return largeWorldCapacity;
  }

  protected getTrackedBodyCapacity(): number {
    return LARGE_WORLD_SPHERES;
  }

  protected async buildScene(): Promise<number[]> {
    this.sphereHandles = [];
    this.largeWorldState = { spheresDropped: 0, scale: largeWorldLiveScale };
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
    this.largeWorldState = { spheresDropped: 0, scale: largeWorldLiveScale };
  }
}

new LargeWorldWorker();
