import type { Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  createSensorVisitScene,
  createSensorVisitState,
  processSensorVisitPostStep,
  sensorVisitGroundSize,
  type SensorVisitState,
} from "./sensor-visit-scene";

class SensorVisitWorker extends PhysicsWorkerBase {
  private sensorVisit: SensorVisitState | null = null;

  protected setupGround(): void {
    // Upstream has no physics ground in Sensor Visit, only DrawGroundGrid(10).
  }

  protected getGroundSize(): Vec3 {
    return sensorVisitGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const scene = createSensorVisitScene(this.world!, this.runtime!);
    this.sensorVisit = createSensorVisitState(scene);
    // Live render order tracks the persistent sensor first so the visitor can be hidden after destruction.
    return [scene.sensorBody, scene.visitorBody];
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null) return;
    this.world.step(this.fixedTimeStep, this.subSteps);
    if (this.sensorVisit !== null && processSensorVisitPostStep(this.world, this.sensorVisit)) {
      this.setTrackedBodies([this.sensorVisit.sensorBody]);
    }
    this.totalSteps += 1;
  }
}

new SensorVisitWorker();
