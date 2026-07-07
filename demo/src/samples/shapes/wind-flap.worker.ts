import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildWindFlapDynamicBodies, dumpPostStep, windFlapGroundSize, type WindFlapState } from "./wind-flap-scene";

class WindFlapWorker extends PhysicsWorkerBase {
  private flapState: WindFlapState | null = null;

  protected getGroundSize(): Vec3 { return windFlapGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    const built = buildWindFlapDynamicBodies(this.world!, this.runtime!);
    this.flapState = built.state;
    return built.handles;
  }

  protected stepPhysics(): number {
    if (this.world === null) return 0;
    const start = performance.now();
    this.world.step(this.fixedTimeStep, this.subSteps);
    if (this.flapState !== null && this.runtime !== null) {
      dumpPostStep(this.world, this.runtime, [], this.totalSteps + 1, this.fixedTimeStep, this.flapState);
    }
    return performance.now() - start;
  }
}

new WindFlapWorker();
