import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyHandle, HullHandle, MeshHandle, ShapeHandle, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  BIG_BOX_DEFAULT_SCALE,
  type BigBoxShapeType,
  bigBoxGroundSize,
  buildBigBoxGround,
  spawnBigBoxBody,
} from "./big-box-scene";

function parseShapeType(value: unknown): BigBoxShapeType | null {
  if (value === "sphere" || value === "capsule" || value === "box" || value === "cylinder") return value;
  return null;
}

class BigBoxWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private meshShape: ShapeHandle | null = null;
  private scale: Vec3 = [...BIG_BOX_DEFAULT_SCALE];
  private shapeType: BigBoxShapeType = "cylinder";
  private dynamic: BodyHandle | null = null;
  private cylinderHull: HullHandle | null = null;

  protected setupGround(): void {
    const built = buildBigBoxGround(this.world!, this.scale);
    this.mesh = built.mesh;
    this.meshShape = built.shape;
  }

  protected getGroundSize(): Vec3 {
    return bigBoxGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    this.cylinderHull = this.runtime!.createCylinder(0.3, 0.15, 0, 32);
    this.dynamic = spawnBigBoxBody(this.world!, this.runtime!, this.shapeType, this.cylinderHull);
    return [this.dynamic];
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
    this.dynamic = spawnBigBoxBody(this.world!, this.runtime!, this.shapeType, this.cylinderHull);
    this.setTrackedBodies([this.dynamic]);
  }

  private applyScale(): void {
    if (this.mesh === null || this.meshShape === null) return;
    this.world!.setMesh(this.meshShape, this.mesh, this.scale);
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null) {
      this.world!.destroyMesh(this.mesh);
      this.mesh = null;
    }
    if (this.cylinderHull !== null && this.runtime !== null) {
      this.runtime.destroyHull(this.cylinderHull);
      this.cylinderHull = null;
    }
    this.meshShape = null;
    this.dynamic = null;
  }
}

new BigBoxWorker();
