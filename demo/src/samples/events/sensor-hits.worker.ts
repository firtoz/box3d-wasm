import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildSensorHitsDynamicBodies,
  sensorHitsGroundSize,
  sensorHitsPreStep,
  type SensorHitsState,
} from "./sensor-hits-scene";

class SensorHitsWorker extends PhysicsWorkerBase {
  private sensorHits: SensorHitsState | null = null;
  private mesh: MeshHandle | null = null;

  protected getGroundSize(): Vec3 {
    return sensorHitsGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { handles, state } = buildSensorHitsDynamicBodies(this.world!, this.runtime!);
    this.sensorHits = state;
    this.mesh = state.mesh;
    return handles;
  }

  protected stepPhysics(): void {
    if (this.world === null || this.sensorHits === null) return;
    sensorHitsPreStep(this.world, this.sensorHits);
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
    this.sensorHits = null;
  }
}

new SensorHitsWorker();
