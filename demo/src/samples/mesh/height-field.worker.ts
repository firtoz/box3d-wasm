import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { HeightFieldHandle, Vec3 } from "box3d-wasm";
import { createHeightFieldScene, heightFieldGroundSize } from "./height-field-scene";

class HeightFieldWorker extends PhysicsWorkerBase {
  private heightField: HeightFieldHandle | null = null;

  protected setupGround(): void {
    // Heightfield ground is created in createHeightFieldScene.
  }

  protected getGroundSize(): Vec3 {
    return heightFieldGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { heightField } = createHeightFieldScene(this.world!, this.runtime!);
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

new HeightFieldWorker();
