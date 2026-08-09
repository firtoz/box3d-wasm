import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildFarMeshDrop, farMeshDropGroundSize } from "./far-mesh-drop-scene";

class FarMeshDropWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Wave mesh ground is created in createMeshDrop.
  }

  protected getGroundSize(): Vec3 {
    return farMeshDropGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { bodies, mesh } = buildFarMeshDrop(this.world!, this.runtime!);
    this.mesh = mesh;
    return bodies;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new FarMeshDropWorker();
