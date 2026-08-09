import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { HeightFieldHandle, Vec3 , BodyId} from "box3d-wasm";
import { buildDrivingScene, drivingGroundSize } from "./driving-scene";

class DrivingWorker extends PhysicsWorkerBase {
  private heightField: HeightFieldHandle | null = null;

  protected setupGround(): void {
    // Wave heightfield ground is created in buildDrivingScene.
  }

  protected getGroundSize(): Vec3 {
    return drivingGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const scene = buildDrivingScene(this.world!, this.runtime!);
    this.heightField = scene.heightField;
    return scene.handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.heightField !== null && this.world !== null) {
      this.world.destroyHeightField(this.heightField);
      this.heightField = null;
    }
  }
}

new DrivingWorker();
