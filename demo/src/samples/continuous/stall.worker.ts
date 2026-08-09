import type { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildStallScene, createStallBullet, stallGroundSize } from "./stall-scene";

class StallWorker extends PhysicsWorkerBase {
  private torusId: BodyId | 0n = 0n;
  private bulletId: BodyId | 0n = 0n;
  private savedThreshold: number | null = null;

  protected getGroundSize(): Vec3 {
    return stallGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { handles, savedThreshold } = buildStallScene(this.world!, this.runtime!);
    this.torusId = handles[0] as BodyId;
    this.bulletId = handles[1] as BodyId;
    this.savedThreshold = savedThreshold;
    return handles;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type !== "launch") return false;
    if (this.world === null || this.runtime === null || this.torusId === 0n) return false;

    if (this.bulletId !== 0n) {
      this.setTrackedBodies([this.torusId]);
      this.world.destroyBody(this.bulletId);
      this.bulletId = 0n;
    }

    this.bulletId = createStallBullet(this.world, this.runtime);
    this.setTrackedBodies([this.torusId, this.bulletId]);
    return true;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.savedThreshold !== null && this.runtime !== null) {
      this.runtime.setStallThreshold(this.savedThreshold);
      this.savedThreshold = null;
    }
    this.torusId = 0n;
    this.bulletId = 0n;
  }
}

new StallWorker();
