import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyId, HullHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { attachConveyorMesh, buildConveyorCylinders, conveyorMeshGroundSize } from "./conveyor-mesh-scene";

class ConveyorMeshWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];
  private hull: HullHandle | null = null;

  protected getGroundSize(): Vec3 {
    return conveyorMeshGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const attached = await attachConveyorMesh(this.world!, this.runtime!);
    const { handles, hull } = buildConveyorCylinders(this.world!, this.runtime!);
    this.hull = hull;
    if (attached !== null) {
      this.meshes.push(attached.mesh);
      return [attached.body, ...handles];
    }
    return handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.hull !== null) this.runtime!.destroyHull(this.hull);
    this.hull = null;
    if (this.world !== null) {
      for (let i = this.meshes.length - 1; i >= 0; i--) this.world.destroyMesh(this.meshes[i]!);
    }
    this.meshes = [];
  }
}

new ConveyorMeshWorker();
