import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildMeshDropDeterminism, meshDropDeterminismGroundSize } from "./mesh-drop-scene";

class MeshDropDeterminismWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Wave mesh ground is created in createMeshDrop.
  }

  protected getGroundSize(): Vec3 {
    return meshDropDeterminismGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { bodies, mesh } = buildMeshDropDeterminism(this.world!, this.runtime!);
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

new MeshDropDeterminismWorker();
