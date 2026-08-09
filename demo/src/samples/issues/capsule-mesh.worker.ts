import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 , BodyId} from "box3d-wasm";
import { buildCapsuleMeshDynamicBodiesAsync, capsuleMeshGroundSize } from "./capsule-mesh-scene";

class CapsuleMeshWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];

  protected setupGround(): void {
    // Ground hull is created in buildCapsuleMeshScene.
  }

  protected getGroundSize(): Vec3 {
    return capsuleMeshGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { handles, meshes } = await buildCapsuleMeshDynamicBodiesAsync(this.world!, this.runtime!);
    this.meshes = meshes;
    return handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world !== null) {
      for (let i = this.meshes.length - 1; i >= 0; i--) {
        this.world.destroyMesh(this.meshes[i]!);
      }
    }
    this.meshes = [];
  }
}

new CapsuleMeshWorker();
