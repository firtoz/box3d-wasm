import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  CONTINUOUS_MESH_DROP_SEED,
  CONTINUOUS_MESH_DROP_SHAPE_TYPES,
  createContinuousMeshDrop,
  continuousMeshDropGroundSize,
  type ContinuousMeshDropShapeType,
} from "./mesh-drop-scene";

function parseShapeType(value: unknown): ContinuousMeshDropShapeType | null {
  if (typeof value !== "string") return null;
  return CONTINUOUS_MESH_DROP_SHAPE_TYPES.find((entry) => entry === value) ?? null;
}

class ContinuousMeshDropWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private shapeType: ContinuousMeshDropShapeType = "box";
  private amplitude = 0.5;
  private collide = true;
  private seed = CONTINUOUS_MESH_DROP_SEED;

  protected setupGround(): void {
    // Wave mesh + walls created in createContinuousMeshDrop.
  }

  protected getGroundSize(): Vec3 {
    return continuousMeshDropGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { ground, bodies, mesh } = createContinuousMeshDrop(this.world!, this.runtime!, {
      seed: this.seed,
      amplitude: this.amplitude,
      collide: this.collide,
      shapeType: this.shapeType,
    });
    this.mesh = mesh;
    return [ground, ...bodies];
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "set-shape-type") {
      const next = parseShapeType(msg.shape);
      if (next === null || next === this.shapeType) return true;
      this.shapeType = next;
      void this.restartScene();
      return true;
    }
    if (msg.type === "set-amplitude" && typeof msg.value === "number") {
      this.amplitude = msg.value;
      void this.restartScene();
      return true;
    }
    if (msg.type === "set-collide") {
      this.collide = msg.value === true;
      void this.restartScene();
      return true;
    }
    if (msg.type === "generate") {
      const seed = typeof msg.seed === "number" ? msg.seed >>> 0 : (performance.now() * 1000) >>> 0;
      this.seed = seed;
      void this.restartScene();
      return true;
    }
    return false;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new ContinuousMeshDropWorker();
