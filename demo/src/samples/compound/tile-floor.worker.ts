import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildTileFloorDynamicBodies, tileFloorGroundSize } from "./tile-floor-scene";

class TileFloorWorker extends PhysicsWorkerBase {
  protected setupGround(): void {}

  protected getGroundSize(): Vec3 { return tileFloorGroundSize(); }

  protected async buildScene(): Promise<number[]> {
    return buildTileFloorDynamicBodies(this.world!, this.runtime!);
  }
}

new TileFloorWorker();
