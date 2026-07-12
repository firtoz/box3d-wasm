import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildExplosionDynamicBodies, buildExplosionGround, EXPLOSION_IMPULSE, explosionGroundSize } from "./explosion-scene";

class ExplosionWorker extends PhysicsWorkerBase {
  private impulse = EXPLOSION_IMPULSE;

  protected setupGround(): void {
    buildExplosionGround(this.world!, this.runtime!);
  }

  protected getGroundSize(): Vec3 {
    return explosionGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildExplosionDynamicBodies(this.world!, this.runtime!);
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "set-explosion-magnitude") {
      if (typeof msg.value === "number") this.impulse = msg.value;
      return true;
    }
    if (msg.type !== "explode") return false;
    const impulse = typeof msg.impulse === "number" ? msg.impulse : this.impulse;
    this.world!.explode([0, -4, 0], 16, 0, impulse, 0xFFFFFFFFn as unknown as number);
    return true;
  }
}

new ExplosionWorker();
