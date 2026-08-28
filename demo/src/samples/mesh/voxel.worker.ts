import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { buildVoxelLive, voxelGroundSize } from "./voxel-scene";

class VoxelWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return voxelGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { handles, meshes } = await buildVoxelLive(this.world!, this.runtime!);
    this.meshes = meshes;
    return handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world !== null) {
      for (let i = this.meshes.length - 1; i >= 0; i--) this.world.destroyMesh(this.meshes[i]!);
    }
    this.meshes = [];
  }
}

new VoxelWorker();
