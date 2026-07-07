import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildSpinningStickDynamicBodies, spinningStickGroundSize } from "./spinning-stick-scene";

class SpinningStickWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return spinningStickGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildSpinningStickDynamicBodies(this.world!, this.runtime!); }
}

new SpinningStickWorker();
