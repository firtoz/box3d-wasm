import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildRainDynamicBodies, rainGroundSize, stepRain } from "./rain-scene";

class RainWorker extends PhysicsWorkerBase {
  private handles: number[] = [];
  private rainState = { columnCount: 0 };

  protected getGroundSize(): Vec3 { return rainGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    this.handles = buildRainDynamicBodies(this.world!, this.runtime!);
    return this.handles;
  }

  protected stepPhysics(): number {
    stepRain(this.world!, this.runtime!, this.handles, this.totalSteps, this.rainState);
    return super.stepPhysics();
  }
}

new RainWorker();
