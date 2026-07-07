import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildWindDynamicBodies, dumpPostStep, windGroundSize, type WindState } from "./wind-scene";

class WindWorker extends PhysicsWorkerBase {
  private windState: WindState | null = null;

  protected getGroundSize(): Vec3 { return windGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    const built = buildWindDynamicBodies(this.world!, this.runtime!);
    this.windState = built.state;
    return built.handles;
  }

  protected stepPhysics(): number {
    if (this.world === null) return 0;
    const start = performance.now();
    this.world.step(this.fixedTimeStep, this.subSteps);
    if (this.windState !== null && this.runtime !== null) {
      dumpPostStep(this.world, this.runtime, [], this.totalSteps + 1, this.fixedTimeStep, this.windState);
    }
    return performance.now() - start;
  }
}

new WindWorker();
