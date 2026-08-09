import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildSpinningStickDynamicBodies, spinningStickGroundSize } from "./spinning-stick-scene";

class SpinningStickWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return spinningStickGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildSpinningStickDynamicBodies(this.world!, this.runtime!); }
}

new SpinningStickWorker();
