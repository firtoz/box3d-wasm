import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 } from "box3d-wasm";
import { buildPersistentContactScene, persistentContactGroundSize } from "./persistent-contact-scene";

class PersistentContactWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Grid mesh ground is created in buildPersistentContactScene.
  }

  protected getGroundSize(): Vec3 {
    return persistentContactGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { sphere, mesh } = buildPersistentContactScene(this.world!, this.runtime!);
    this.mesh = mesh;
    return [sphere];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new PersistentContactWorker();
