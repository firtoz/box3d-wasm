import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildThinWallDynamicBodies, thinWallGroundSize } from "./thin-wall-scene";

class ThinWallWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return thinWallGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildThinWallDynamicBodies(this.world!, this.runtime!); }
}

new ThinWallWorker();
