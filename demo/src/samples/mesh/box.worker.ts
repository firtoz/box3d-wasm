import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyHandle, MeshHandle, ShapeHandle, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildMeshBoxDynamicBodies,
  buildMeshBoxGroundBox,
  buildMeshBoxPlatform,
  MESH_BOX_DEFAULT_SCALE,
  type MeshBoxShapeType,
  meshBoxGroundSize,
  spawnMeshBoxBody,
} from "./box-scene";

function parseShapeType(value: unknown): MeshBoxShapeType | null {
  if (value === "sphere" || value === "capsule" || value === "box" || value === "cylinder") return value;
  return null;
}

class MeshBoxWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private meshShape: ShapeHandle | null = null;
  private scale: Vec3 = [...MESH_BOX_DEFAULT_SCALE];
  private shapeType: MeshBoxShapeType = "box";
  private dynamic: BodyHandle | null = null;
  private platform: BodyHandle | null = null;

  protected setupGround(): void {
    buildMeshBoxGroundBox(this.world!, this.runtime!);
  }

  protected getGroundSize(): Vec3 {
    return meshBoxGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    const built = buildMeshBoxPlatform(this.world!, this.runtime!, this.scale);
    this.platform = built.platform;
    this.mesh = built.mesh;
    this.meshShape = built.shape;
    const handles = buildMeshBoxDynamicBodies(this.world!, this.runtime!);
    this.dynamic = handles[0] ?? null;
    return [this.platform, ...handles];
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "set-mesh-scale-x" && typeof msg.value === "number") {
      this.scale = [msg.value, this.scale[1], this.scale[2]];
      this.applyScale();
      return true;
    }
    if (msg.type === "set-mesh-scale-z" && typeof msg.value === "number") {
      this.scale = [this.scale[0], this.scale[1], msg.value];
      this.applyScale();
      return true;
    }
    if (msg.type === "set-shape") {
      const next = parseShapeType(msg.shape);
      if (next === null) return true;
      this.shapeType = next;
      this.respawn();
      return true;
    }
    return false;
  }

  private respawn(): void {
    if (this.dynamic !== null) this.world!.destroyBody(this.dynamic);
    this.dynamic = spawnMeshBoxBody(this.world!, this.runtime!, this.shapeType);
    this.setTrackedBodies(this.platform !== null ? [this.platform, this.dynamic] : [this.dynamic]);
  }

  private applyScale(): void {
    if (this.mesh === null || this.meshShape === null) return;
    this.world!.setMesh(this.meshShape, this.mesh, this.scale);
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
    this.meshShape = null;
    this.dynamic = null;
    this.platform = null;
  }
}

new MeshBoxWorker();
