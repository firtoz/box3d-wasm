import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildMeshTileDynamicBodies, meshTileGroundSize } from "./mesh-tile-scene";

class MeshTileWorker extends PhysicsWorkerBase {
  protected setupGround(): void {
    // Upstream Mesh Tile creates only the baked compound body; no AddGroundBox.
  }

  protected getGroundSize(): Vec3 {
    return meshTileGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildMeshTileDynamicBodies(this.world!, this.runtime!);
  }
}

new MeshTileWorker();
