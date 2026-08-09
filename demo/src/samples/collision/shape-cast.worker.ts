import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 , BodyId} from "box3d-wasm";
import { buildShapeCastScene, shapeCastGroundSize } from "./shape-cast-scene";

class ShapeCastWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Upstream has no physics ground — static targets only.
  }

  protected getWorldGravity(): Vec3 {
    return [0, 0, 0];
  }

  protected getGroundSize(): Vec3 {
    return shapeCastGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const mesh = this.world!.createTorusMesh(10, 12, 0.65, 0.35);
    this.mesh = mesh;
    return buildShapeCastScene(this.world!, this.runtime!, mesh);
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new ShapeCastWorker();
