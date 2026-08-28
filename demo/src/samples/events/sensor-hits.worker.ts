import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
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

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    const state = this.sensorHits;
    if (world === null || state === null) return false;
    if (msg.type === "set-bullet" && typeof msg.value === "boolean") {
      world.setBodyBullet(state.launchBody, msg.value);
      return true;
    }
    if (msg.type === "launch") {
      const speed = 200 + Math.random() * 100;
      world.setBodyTransform(state.launchBody, [-26.7, 6, 0]);
      world.setBodyLinearVelocity(state.launchBody, [speed, 0, 0]);
      world.setBodyAngularVelocity(state.launchBody, [0, 0, 0]);
      world.setBodyAwake(state.launchBody, true);
      return true;
    }
    return false;
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
