import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyId, BodyTransform, Vec3 } from "box3d-wasm";
import {
  BODY_CAST_HUD_FLOATS,
  bodyCastGroundSize,
  bodyCastQueryTransform,
  buildBodyCastDynamicBodies,
  fillBodyCastHud,
} from "./cast-scene";

class BodyCastWorker extends PhysicsWorkerBase {
  private handles: BodyId[] = [];
  private queryXf: BodyTransform | null = null;
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Float32Array | null = null;

  protected setupGround(): void {
    // Upstream has no physics ground — kinematic cylinder only.
  }

  protected getGroundSize(): Vec3 {
    return bodyCastGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { bodyCastHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.queryXf = bodyCastQueryTransform(this.runtime!);
    this.hudBuffer = new SharedArrayBuffer(BODY_CAST_HUD_FLOATS * 4);
    this.hud = new Float32Array(this.hudBuffer);
    this.handles = buildBodyCastDynamicBodies(this.world!, this.runtime!);
    fillBodyCastHud(this.world!, this.handles[0]!, this.queryXf, this.hud);
    return this.handles;
  }

  protected publishExtra(): void {
    if (this.world === null || this.handles.length === 0 || this.queryXf === null || this.hud === null) return;
    fillBodyCastHud(this.world, this.handles[0]!, this.queryXf, this.hud);
  }
}

new BodyCastWorker();
