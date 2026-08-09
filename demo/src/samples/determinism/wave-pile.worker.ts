import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { HeightFieldHandle, Vec3 } from "box3d-wasm";
import { createWavePile, wavePileGroundSize } from "./wave-pile-scene";

class WavePileWorker extends PhysicsWorkerBase {
  private heightField: HeightFieldHandle | null = null;

  protected setupGround(): void {
    // Wave heightfield ground is created in createWavePile.
  }

  protected getGroundSize(): Vec3 {
    return wavePileGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { bodies, heightField } = createWavePile(this.world!, this.runtime!);
    this.heightField = heightField;
    return bodies;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.heightField !== null && this.world !== null) {
      this.world.destroyHeightField(this.heightField);
      this.heightField = null;
    }
  }
}

new WavePileWorker();
