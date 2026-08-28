import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { createJointEventScene, dumpPostStep, jointEventGroundSize } from "./joint-scene";

export const JOINT_EVENT_HUD_INTS = 2;

class JointEventWorker extends PhysicsWorkerBase {
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Int32Array | null = null;
  private breakCount = 0;

  protected getGroundSize(): Vec3 {
    return jointEventGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { jointEventHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.hudBuffer = new SharedArrayBuffer(JOINT_EVENT_HUD_INTS * 4);
    this.hud = new Int32Array(this.hudBuffer);
    this.breakCount = 0;
    return createJointEventScene(this.world!, this.runtime!).bodies;
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null) return;
    this.world.step(this.fixedTimeStep, this.subSteps);
    const broke = this.world.getJointEventHandles();
    this.breakCount += broke.length;
    if (this.hud !== null) {
      this.hud[0] = broke.length;
      this.hud[1] = this.breakCount;
    }
    dumpPostStep(this.world, this.runtime, [], this.totalSteps + 1, this.fixedTimeStep, undefined);
    this.totalSteps += 1;
  }
}

new JointEventWorker();
