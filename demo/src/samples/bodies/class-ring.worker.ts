import {type Vec3, type BodyId} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildClassRingDynamicBodies, classRingGroundSize, stepClassRing } from "./class-ring-scene";

class ClassRingWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return classRingGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildClassRingDynamicBodies(this.world!, this.runtime!);
  }

  protected stepPhysics(): void {
    if (this.world === null) return;
    stepClassRing(this.world);
    this.totalSteps += 1;
  }
}

new ClassRingWorker();
