import type { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  EXTRA_ONE_WAY_HUD_INTS,
  buildExtraOneWayDynamicBodies,
  extraOneWayGroundSize,
  extraOneWayHandleIndex,
} from "./one-way-scene";

class ExtraOneWayWorker extends PhysicsWorkerBase {
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Int32Array | null = null;
  private handles: BodyId[] = [];

  protected getGroundSize(): Vec3 {
    return extraOneWayGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { oneWayHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.hudBuffer = new SharedArrayBuffer(EXTRA_ONE_WAY_HUD_INTS * 4);
    this.hud = new Int32Array(this.hudBuffer);
    this.handles = buildExtraOneWayDynamicBodies(this.world!, this.runtime!);
    return this.handles;
  }

  protected publishExtra(): void {
    if (this.world === null || this.hud === null) return;
    const stats = this.world.getPreSolveStats();
    this.hud[0] = stats.keep;
    this.hud[1] = stats.skip;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "launch") {
      const body = this.handles[extraOneWayHandleIndex.launch];
      if (body !== undefined) {
        this.world?.setBodyTransform(body, [0, 0.5, 0]);
        this.world?.setBodyLinearVelocity(body, [0, 14, 0]);
        this.world?.setBodyAngularVelocity(body, [0, 0, 0]);
      }
      return true;
    }
    if (msg.type === "drop") {
      const body = this.handles[extraOneWayHandleIndex.drop];
      if (body !== undefined) {
        this.world?.setBodyTransform(body, [0, 6, 0]);
        this.world?.setBodyLinearVelocity(body, [0, 0, 0]);
        this.world?.setBodyAngularVelocity(body, [0, 0, 0]);
      }
      return true;
    }
    return false;
  }
}

new ExtraOneWayWorker();
