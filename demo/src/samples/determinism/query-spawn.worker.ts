import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import {
  QUERY_SPAWN_COUNT,
  QUERY_SPAWN_SEED,
  createQuerySpawnState,
  querySpawnDumpStep,
  querySpawnGroundSize,
  type QuerySpawnState,
} from "./query-spawn-scene";

class QuerySpawnWorker extends PhysicsWorkerBase {
  private queryState: QuerySpawnState = createQuerySpawnState();

  protected setupGround(): void {
    // Upstream CreateQuerySpawn has no ground — empty zero-g space.
  }

  protected getWorldGravity(): Vec3 {
    return [0, 0, 0];
  }

  protected getGroundSize(): Vec3 {
    return querySpawnGroundSize();
  }

  protected getTrackedBodyCapacity(): number {
    return QUERY_SPAWN_COUNT;
  }

  protected async buildScene(): Promise<number[]> {
    this.runtime!.setRandomSeed(QUERY_SPAWN_SEED);
    this.queryState = createQuerySpawnState();
    return this.queryState.bodies;
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null) return;
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
    const before = this.queryState.bodies.length;
    querySpawnDumpStep(this.world, this.runtime, [], this.totalSteps, this.fixedTimeStep, this.queryState);
    if (this.queryState.bodies.length !== before) {
      this.setTrackedBodies(this.queryState.bodies);
    }
  }
}

new QuerySpawnWorker();
