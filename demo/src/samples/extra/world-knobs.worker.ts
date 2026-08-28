import { OverlapProxyType, type BodyId, type Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  EXTRA_WORLD_KNOBS_HUD_FLOATS,
  buildExtraWorldKnobsDynamicBodies,
  extraWorldKnobsGroundSize,
} from "./world-knobs-scene";

class ExtraWorldKnobsWorker extends PhysicsWorkerBase {
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Float32Array | null = null;
  private gravityY = -10;
  private speculative = true;

  protected getGroundSize(): Vec3 {
    return extraWorldKnobsGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { worldKnobsHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.hudBuffer = new SharedArrayBuffer(EXTRA_WORLD_KNOBS_HUD_FLOATS * 4);
    this.hud = new Float32Array(this.hudBuffer);
    this.world!.enableSpeculative(this.speculative);
    return buildExtraWorldKnobsDynamicBodies(this.world!, this.runtime!);
  }

  protected publishExtra(): void {
    if (this.world === null || this.hud === null) return;
    const g = this.world.getGravity();
    this.hud[0] = g[0];
    this.hud[1] = g[1];
    this.hud[2] = g[2];
    this.hud[3] = this.world.getRestitutionThreshold();
    this.hud[4] = this.world.getHitEventThreshold();
    this.hud[5] = this.world.getMaximumLinearSpeed();
    this.hud[6] = this.world.getContactRecycleDistance();
    this.hud[7] = this.world.overlapShape([0, 2, 0], { type: OverlapProxyType.Sphere, radius: 2 }).length;
    const counters = this.world.getCounters();
    this.hud[8] = counters.staticTreeHeight;
    this.hud[9] = counters.treeHeight;
    this.hud[10] = counters.bodyCount;
    this.hud[11] = counters.contactCount;
    this.hud[12] = this.world.getWorkerCount();
    this.hud[13] = this.runtime!.getStallThreshold();
    const profile = this.world.getProfileLevel();
    this.hud[14] = profile === "off" ? 0 : profile === "coarse" ? 1 : 2;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "setGravityY" && typeof msg.value === "number") {
      this.gravityY = msg.value;
      this.world?.setGravity([0, this.gravityY, 0]);
      return true;
    }
    if (msg.type === "setSpeculative") {
      this.speculative = msg.value === true;
      this.world?.enableSpeculative(this.speculative);
      return true;
    }
    if (msg.type === "rebuildStaticTree") {
      this.world?.rebuildStaticTree();
      return true;
    }
    if (msg.type === "setProfileLevel" && typeof msg.value === "number") {
      const level = msg.value <= 0 ? "off" : msg.value === 1 ? "coarse" : "full";
      this.world?.setProfileLevel(level);
      return true;
    }
    return false;
  }
}

new ExtraWorldKnobsWorker();
