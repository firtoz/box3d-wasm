import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyHandle, MeshHandle, ShapeHandle, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildMeshGridDynamicBodies,
  buildMeshGridGround,
  MESH_GRID_DEFAULT_SCALE,
  type MeshGridShapeType,
  meshGridGroundSize,
  spawnMeshGridBody,
} from "./grid-scene";

function parseShapeType(value: unknown): MeshGridShapeType | null {
  if (value === "sphere" || value === "capsule" || value === "box" || value === "cylinder") return value;
  return null;
}

class MeshGridWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private meshShape: ShapeHandle | null = null;
  private scale: Vec3 = [...MESH_GRID_DEFAULT_SCALE];
  private shapeType: MeshGridShapeType = "cylinder";
  private dynamic: BodyHandle | null = null;

  protected setupGround(): void {
    const built = buildMeshGridGround(this.world!, this.scale);
    this.mesh = built.mesh;
    this.meshShape = built.shape;
  }

  protected getGroundSize(): Vec3 {
    return meshGridGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    const handles = buildMeshGridDynamicBodies(this.world!, this.runtime!);
    this.dynamic = handles[0] ?? null;
    return handles;
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
    this.dynamic = spawnMeshGridBody(this.world!, this.runtime!, this.shapeType);
    this.setTrackedBodies([this.dynamic]);
  }

  private applyScale(): void {
    if (this.mesh === null || this.meshShape === null) return;
    this.world!.setMesh(this.meshShape, this.mesh, this.scale);
  }
}

new MeshGridWorker();
