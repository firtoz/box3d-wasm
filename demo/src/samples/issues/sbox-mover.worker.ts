import type { BodyId, HeightFieldHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildSboxMoverDynamicBodies, sboxMoverGroundSize } from "./sbox-mover-scene";

class SboxMoverWorker extends PhysicsWorkerBase {
  private heightField: HeightFieldHandle | null = null;
  private meshes: MeshHandle[] = [];

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return sboxMoverGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const built = buildSboxMoverDynamicBodies(this.world!, this.runtime!);
    this.heightField = built.heightField;
    this.meshes = built.meshes;
    return built.handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world === null) return;
    for (let i = this.meshes.length - 1; i >= 0; i--) this.world.destroyMesh(this.meshes[i]!);
    if (this.heightField !== null) this.world.destroyHeightField(this.heightField);
    this.meshes = [];
    this.heightField = null;
  }
}

new SboxMoverWorker();
