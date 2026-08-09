import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 } from "box3d-wasm";
import { buildMeshScaleScene, meshScaleGroundSize } from "./mesh-scale-scene";

class MeshScaleWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Upstream has no physics ground.
  }

  protected getGroundSize(): Vec3 {
    return meshScaleGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { body, mesh } = buildMeshScaleScene(this.world!, this.runtime!);
    this.mesh = mesh;
    return [body];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new MeshScaleWorker();
