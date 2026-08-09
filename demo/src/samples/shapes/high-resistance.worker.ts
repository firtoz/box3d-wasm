import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildHighResistanceDynamicBodies, highResistanceGroundSize } from "./high-resistance-scene";

class HighResistanceWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return highResistanceGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildHighResistanceDynamicBodies(this.world!, this.runtime!); }
}

new HighResistanceWorker();
