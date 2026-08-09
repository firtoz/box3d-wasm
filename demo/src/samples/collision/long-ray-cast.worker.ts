import { BodyId, HeightFieldHandle, HullHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildLongRayCastScene, longRayCastGroundSize } from "./long-ray-cast-scene";

class LongRayCastWorker extends PhysicsWorkerBase {
  private hull: HullHandle | null = null;
  private mesh: MeshHandle | null = null;
  private heightField: HeightFieldHandle | null = null;

  protected setupGround(): void {
    // Upstream has no AddGroundBox — only the five static targets.
  }

  protected getWorldGravity(): Vec3 {
    return [0, 0, 0];
  }

  protected getGroundSize(): Vec3 {
    return longRayCastGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { handles, resources } = buildLongRayCastScene(this.world!, this.runtime!);
    this.hull = resources.hull;
    this.mesh = resources.mesh;
    this.heightField = resources.heightField;
    return handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world === null || this.runtime === null) return;
    if (this.hull !== null) {
      this.runtime.destroyHull(this.hull);
      this.hull = null;
    }
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

new LongRayCastWorker();
