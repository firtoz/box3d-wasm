import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type Quat, type Vec3 } from "box3d-wasm";

class KinematicWorker extends PhysicsWorkerBase {
  private kinematicTime = 0;
  private kinematicBodyId = 0;
  private readonly amplitude = 2;

  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const halfWidths: Vec3 = [0.05, 0.5, 0.1];

    const body = this.world!.createBody({
      type: 1, position: [2 * this.amplitude, this.amplitude + 1, 0],
    });
    this.runtime!.createHullShape(body, halfWidths, {});
    this.kinematicBodyId = body;
    return [body];
  }

  protected stepPhysics(): number {
    const timeStep = this.fixedTimeStep;
    const delay = 2;

    if (timeStep > 0 && this.kinematicTime > delay) {
      const t = this.kinematicTime - delay;
      const point: Vec3 = [
        2 * this.amplitude * Math.cos(t),
        this.amplitude * (Math.sin(2 * t) + 1) + 1,
        0,
      ];
      const a = t;
      const rotation: Quat = [0, 0, Math.sin(a), Math.cos(a)];
      this.runtime!.setBodyTargetTransform(this.kinematicBodyId, point, rotation, timeStep, true);
    }

    const start = performance.now();
    this.world!.step(this.fixedTimeStep, this.subSteps);
    this.kinematicTime += timeStep;
    return performance.now() - start;
  }
}

new KinematicWorker();
