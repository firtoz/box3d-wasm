import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildDisableDynamicBodies, disableGroundSize } from "./disable-scene";

class DisableWorker extends PhysicsWorkerBase {
  private linkIds: number[] = [];
  private ballId = 0;

  protected getGroundSize(): Vec3 {
    return disableGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const handles = buildDisableDynamicBodies(this.world!, this.runtime!);
    // handles[0..3] = links 0..3, handles[4] = ball
    this.linkIds = [handles[0], handles[1], handles[2], handles[3]];
    this.ballId = handles[4];
    return handles;
  }

  protected stepPhysics(): number {
    const start = performance.now();
    this.world!.step(this.fixedTimeStep, this.subSteps);
    return performance.now() - start;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const R = this.runtime!;
    switch (msg.type) {
      case "enableLink2": {
        if (msg.value) { R.bodyEnable(this.linkIds[2]); } else { R.bodyDisable(this.linkIds[2]); }
        return true;
      }
      case "enableBall": {
        if (msg.value) { R.bodyEnable(this.ballId); } else { R.bodyDisable(this.ballId); }
        return true;
      }
    }
    return false;
  }
}

new DisableWorker();
