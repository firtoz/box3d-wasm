import { PhysicsWorkerBase } from "../../physics-worker-base";
import { type BodyHandle, type Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildWeebleDynamicBodies, explodeWeeble, teleportWeeble, weebleGroundSize } from "./weeble-scene";

class WeebleWorker extends PhysicsWorkerBase {
  private weebleId: BodyHandle | null = null;
  private debugBuffer: SharedArrayBuffer | null = null;
  private debugValues: Float32Array | null = null;
  private explosionMagnitude = 20000;

  protected getGroundSize(): Vec3 {
    return weebleGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    const handles = buildWeebleDynamicBodies(this.world!, this.runtime!);
    this.weebleId = handles[0];
    this.debugBuffer = new SharedArrayBuffer(10 * 4);
    this.debugValues = new Float32Array(this.debugBuffer);
    return handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.debugBuffer === null ? {} : { debug: this.debugBuffer };
  }

  protected publishExtra(): void {
    if (this.world === null || this.weebleId === null || this.debugValues === null) return;
    const localPoint: Vec3 = [0, 2, 0];
    const worldPoint = this.world.getBodyWorldPoint(this.weebleId, localPoint);
    const localVelocity = this.world.getBodyLocalPointVelocity(this.weebleId, localPoint);
    const worldVelocity = this.world.getBodyWorldPointVelocity(this.weebleId, worldPoint);
    this.debugValues.set([...worldPoint, ...localVelocity, ...worldVelocity, this.explosionMagnitude]);
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
        explodeWeeble(W, this.explosionMagnitude);
        return true;
      }
      case "set-explosion-magnitude": {
        if (typeof msg.value !== "number") return false;
        this.explosionMagnitude = msg.value;
        return true;
      }
    }
    return false;
  }
}

new WeebleWorker();
