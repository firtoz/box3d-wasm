import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type BodyHandle, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildWeebleDynamicBodies, explodeWeeble, teleportWeeble, weebleGroundSize } from "./weeble-scene";

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
        teleportWeeble(R, this.weebleId);
        return true;
      }
      case "explode": {
        explodeWeeble(W);
        return true;
      }
    }
    return false;
  }
}

new WeebleWorker();
