import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type BodyHandle, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildWeebleDynamicBodies, weebleGroundSize } from "./weeble-scene";

class WeebleWorker extends PhysicsWorkerBase {
  private weebleId: BodyHandle | null = null;

  protected getGroundSize(): Vec3 {
    return weebleGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    const handles = buildWeebleDynamicBodies(this.world!, this.runtime!);
    this.weebleId = handles[0];
    return handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const R = this.runtime!;
    const W = this.world!;
    switch (msg.type) {
      case "teleport": {
        if (this.weebleId === null) return false;
        R.setBodyTransform(this.weebleId, [0, 20, 0], [0, 0, 0, 1]);
        R.setBodyAwake(this.weebleId, true);
        return true;
      }
      case "explode": {
        W.explode([0, 0, 0], 30, 2, 10);
        return true;
      }
    }
    return false;
  }
}

new WeebleWorker();
