import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 } from "box3d-wasm";
import { createContinuousMeshDrop, continuousMeshDropGroundSize } from "./mesh-drop-scene";

class ContinuousMeshDropWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Wave mesh + walls created in createContinuousMeshDrop.
  }

  protected getGroundSize(): Vec3 {
    return continuousMeshDropGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    // Include ground so wall compound maps to body index 0 (matches render bodies).
    const { ground, bodies, mesh } = createContinuousMeshDrop(this.world!, this.runtime!);
    this.mesh = mesh;
    return [ground, ...bodies];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new ContinuousMeshDropWorker();
