import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { MeshHandle, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildFallingTreesScene,
  fallingTreesGroundSize,
  treeScaleFromCm,
  type TreeScaleCm,
} from "./falling-trees-scene";

class FallingTreesWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private scaleCm: TreeScaleCm = 100;

  protected setupGround(): void {
    // Wave mesh ground is created in buildFallingTreesScene.
  }

  protected getGroundSize(): Vec3 {
    return fallingTreesGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { trees, mesh } = buildFallingTreesScene(
      this.world!,
      this.runtime!,
      treeScaleFromCm(this.scaleCm),
    );
    this.mesh = mesh;
    return trees;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type !== "set-tree-scale") return false;
    const cm = msg.cm;
    if (cm !== 100 && cm !== 50 && cm !== 25) return true;
    this.scaleCm = cm;
    void this.restartScene();
    return true;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
  }
}

new FallingTreesWorker();
