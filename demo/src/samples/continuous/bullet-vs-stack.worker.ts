import { PhysicsWorkerBase } from "../../physics-worker-base";
import {type BodyId, type Vec3} from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildBulletVsStackDynamicBodies,
  bulletVsStackGroundSize,
  launchBullet,
} from "./bullet-vs-stack-scene";

class BulletVsStackWorker extends PhysicsWorkerBase {
  private baseHandles: BodyId[] = [];
  private bulletId: BodyId | 0n = 0n;

  protected getGroundSize(): Vec3 {
    return bulletVsStackGroundSize();
  }

  protected getTrackedBodyCapacity(initialHandles: BodyId[]): number {
    // Stack bodies + one CCD bullet created on Launch.
    return initialHandles.length + 1;
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.baseHandles = buildBulletVsStackDynamicBodies(this.world!, this.runtime!);
    return this.baseHandles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type !== "launch") return false;
    if (this.world === null || this.runtime === null) return false;

    if (this.bulletId !== 0n) {
      this.setTrackedBodies(this.baseHandles);
      this.world.destroyBody(this.bulletId);
      this.bulletId = 0n;
    }

    const scratch: BodyId[] = [];
    this.bulletId = launchBullet(this.world, this.runtime, scratch);
    this.setTrackedBodies([...this.baseHandles, this.bulletId]);
    return true;
  }
}

new BulletVsStackWorker();
