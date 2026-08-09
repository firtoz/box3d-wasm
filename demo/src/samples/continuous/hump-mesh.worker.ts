import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 } from "box3d-wasm";
import { buildHumpMeshScene, humpMeshGroundSize } from "./hump-mesh-scene";

class HumpMeshWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected getGroundSize(): Vec3 {
    return humpMeshGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { dynamic, mesh } = buildHumpMeshScene(this.world!, this.runtime!);
    this.mesh = mesh;
    return [dynamic];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new HumpMeshWorker();
