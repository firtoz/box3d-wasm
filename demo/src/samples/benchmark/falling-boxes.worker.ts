import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildFallingBoxesDynamicBodies, fallingBoxesGroundSize } from "./falling-boxes-scene";

class FallingBoxesWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return fallingBoxesGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildFallingBoxesDynamicBodies(this.world!, this.runtime!); }
}

new FallingBoxesWorker();
