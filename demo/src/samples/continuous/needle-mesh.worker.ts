import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildNeedleMeshScene, needleMeshGroundSize } from "./needle-mesh-scene";

class NeedleMeshWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];

  protected setupGround(): void {
    // Needle meshes are created in buildNeedleMeshScene.
  }

  protected getGroundSize(): Vec3 {
    return needleMeshGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { dynamic, meshes } = buildNeedleMeshScene(this.world!, this.runtime!);
    this.meshes = meshes;
    return [dynamic];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world !== null) {
      for (let i = this.meshes.length - 1; i >= 0; i--) {
        this.world.destroyMesh(this.meshes[i]!);
      }
    }
    this.meshes = [];
  }
}

new NeedleMeshWorker();
