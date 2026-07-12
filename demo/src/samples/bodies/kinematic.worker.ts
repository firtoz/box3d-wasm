import { type BodyHandle, type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildKinematicDynamicBodies, kinematicGroundSize, stepKinematic } from "./kinematic-scene";

class KinematicWorker extends PhysicsWorkerBase {
  private kinematicTime = 0;
  private kinematicBodyId: BodyHandle | null = null;

  protected getGroundSize(): Vec3 {
    return kinematicGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const handles = buildKinematicDynamicBodies(this.world!, this.runtime!);
    this.kinematicBodyId = handles[0];
    return handles;
  }

  protected stepPhysics(): number {
    if (this.kinematicBodyId === null) return 0;
    this.kinematicTime = stepKinematic(this.world!, this.runtime!, this.kinematicBodyId, this.kinematicTime, this.fixedTimeStep);
    const start = performance.now();
    this.world!.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
    return performance.now() - start;
  }
}

new KinematicWorker();
