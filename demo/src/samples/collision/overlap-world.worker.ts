import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { HeightFieldHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { buildOverlapWorldScene, overlapWorldGroundSize } from "./overlap-world-scene";

class OverlapWorldWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private heightField: HeightFieldHandle | null = null;

  protected setupGround(): void {
    // Upstream has no physics ground — static/dynamic targets only.
  }

  protected getWorldGravity(): Vec3 {
    return [0, 0, 0];
  }

  protected getGroundSize(): Vec3 {
    return overlapWorldGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { handles, resources } = buildOverlapWorldScene(this.world!, this.runtime!);
    this.mesh = resources.mesh;
    this.heightField = resources.heightField;
    return handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world === null) return;
    if (this.mesh !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
    if (this.heightField !== null) {
      this.world.destroyHeightField(this.heightField);
      this.heightField = null;
    }
  }
}

new OverlapWorldWorker();
