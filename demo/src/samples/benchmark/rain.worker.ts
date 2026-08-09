import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { RAGDOLL_RENDER_BONE_COUNT } from "../../physics-worker-protocol";
import {
  buildRainDynamicBodies,
  rainGroundSize,
  rainMaxHumanCount,
  rainTileCount,
  stepRain,
} from "./rain-scene";

class RainWorker extends PhysicsWorkerBase {
  private handles: BodyId[] = [];
  private rainState = { columnCount: 0 };

  protected setupGround(): void {
    // Rain tiles are the ground; skip the default hull plane.
  }

  protected getGroundSize(): Vec3 {
    return rainGroundSize();
  }

  protected getTrackedBodyCapacity(): number {
    return rainTileCount() + rainMaxHumanCount() * RAGDOLL_RENDER_BONE_COUNT;
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.handles = buildRainDynamicBodies(this.world!, this.runtime!);
    return this.handles;
  }

  protected stepPhysics(): void {
    const before = this.handles.length;
    stepRain(this.world!, this.runtime!, this.handles, this.totalSteps, this.rainState);
    if (this.handles.length !== before) this.setTrackedBodies(this.handles);
    super.stepPhysics();
  }
}

new RainWorker();
