import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildTileFloorDynamicBodies, tileFloorGroundSize } from "./tile-floor-scene";

class TileFloorWorker extends PhysicsWorkerBase {
  protected setupGround(): void {}

  protected getGroundSize(): Vec3 { return tileFloorGroundSize(); }

  protected async buildScene(): Promise<BodyId[]> {
    return buildTileFloorDynamicBodies(this.world!, this.runtime!);
  }
}

new TileFloorWorker();
