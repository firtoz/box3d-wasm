import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildHitEventScene, hitEventGroundSize } from "./hit-scene";

class HitEventWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Grid mesh ground is created in buildHitEventScene.
  }

  protected getGroundSize(): Vec3 {
    return hitEventGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { bodies, mesh } = buildHitEventScene(this.world!, this.runtime!);
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

new HitEventWorker();
