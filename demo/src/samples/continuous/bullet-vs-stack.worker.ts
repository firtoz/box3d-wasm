import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type BodyHandle, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildBulletVsStackDynamicBodies,
  bulletVsStackGroundSize,
  launchBullet,
} from "./bullet-vs-stack-scene";

class BulletVsStackWorker extends PhysicsWorkerBase {
  private baseHandles: number[] = [];
  private bulletId: BodyHandle | 0 = 0;

  protected getGroundSize(): Vec3 {
    return bulletVsStackGroundSize();
  }

  protected getTrackedBodyCapacity(initialHandles: number[]): number {
    // Stack bodies + one CCD bullet created on Launch.
    return initialHandles.length + 1;
  }

  protected async buildScene(): Promise<number[]> {
    this.baseHandles = buildBulletVsStackDynamicBodies(this.world!, this.runtime!);
    return this.baseHandles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type !== "launch") return false;
    if (this.world === null || this.runtime === null) return false;

    if (this.bulletId !== 0) {
      this.setTrackedBodies(this.baseHandles);
      this.world.destroyBody(this.bulletId);
      this.bulletId = 0;
    }

    const scratch: number[] = [];
    this.bulletId = launchBullet(this.world, this.runtime, scratch);
    this.setTrackedBodies([...this.baseHandles, this.bulletId]);
    return true;
  }
}

new BulletVsStackWorker();
