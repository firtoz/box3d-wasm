import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { HeightFieldHandle, Vec3 } from "box3d-wasm";
import { benchmarkHeightFieldGroundSize, createBenchmarkHeightFieldScene } from "./height-field-scene";

class BenchmarkHeightFieldWorker extends PhysicsWorkerBase {
  private heightField: HeightFieldHandle | null = null;

  protected setupGround(): void {
    // Heightfield ground is created in createBenchmarkHeightFieldScene.
  }

  protected getGroundSize(): Vec3 {
    return benchmarkHeightFieldGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { heightField } = createBenchmarkHeightFieldScene(this.world!);
    this.heightField = heightField;
    return [];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.heightField !== null && this.world !== null) {
      this.world.destroyHeightField(this.heightField);
      this.heightField = null;
    }
  }
}

new BenchmarkHeightFieldWorker();
