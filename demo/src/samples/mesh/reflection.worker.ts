import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import type { BodyId, MeshHandle, ShapeId, Vec3 } from "box3d-wasm";
import { attachBuildingMeshes, buildReflectionCore, loadBuildingObj, reflectionGroundSize } from "./reflection-scene";

class ReflectionWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];
  private meshShape: ShapeId | null = null;
  private buildingMesh: MeshHandle | null = null;
  private scale: Vec3 = [-1, 1, 1];

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return reflectionGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const built = buildReflectionCore(this.world!, this.runtime!);
    this.meshes.push(built.gridMesh);
    try {
      const obj = await loadBuildingObj();
      const attached = attachBuildingMeshes(this.world!, obj.vertices, obj.indices, built.handles[1]!, built.meshBody, this.scale);
      this.buildingMesh = attached.mesh;
      this.meshShape = attached.meshShape;
      this.meshes.push(attached.mesh);
    } catch {
      // Dump/live without the obj still matches C++ CreateMeshData == null.
    }
    return built.handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type !== "set-scale") return false;
    this.scale = [
      typeof msg.x === "number" ? msg.x : this.scale[0],
      typeof msg.y === "number" ? msg.y : this.scale[1],
      typeof msg.z === "number" ? msg.z : this.scale[2],
    ];
    if (this.meshShape !== null && this.buildingMesh !== null) {
      this.world!.setMesh(this.meshShape, this.buildingMesh, this.scale);
    }
    return true;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world !== null) {
      for (let i = this.meshes.length - 1; i >= 0; i--) this.world.destroyMesh(this.meshes[i]!);
    }
    this.meshes = [];
    this.meshShape = null;
    this.buildingMesh = null;
  }
}

new ReflectionWorker();
