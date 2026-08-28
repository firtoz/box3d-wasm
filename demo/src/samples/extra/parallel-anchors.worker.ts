import type { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  EXTRA_PARALLEL_ANCHORS_HUD_INTS,
  buildExtraParallelAnchorsDynamicBodies,
  extraParallelAnchorsGroundSize,
} from "./parallel-anchors-scene";

class ExtraParallelAnchorsWorker extends PhysicsWorkerBase {
  private hudBuffer: SharedArrayBuffer | null = null;
  private hud: Int32Array | null = null;

  protected getGroundSize(): Vec3 {
    return extraParallelAnchorsGroundSize();
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.hudBuffer === null ? {} : { parallelAnchorsHud: this.hudBuffer };
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.hudBuffer = new SharedArrayBuffer(EXTRA_PARALLEL_ANCHORS_HUD_INTS * 4);
    this.hud = new Int32Array(this.hudBuffer);
    const scene = buildExtraParallelAnchorsDynamicBodies(this.world!, this.runtime!);
    const boneCount = this.runtime!.getHumanBoneCount();
    let anchors = 0;
    for (let i = 0; i < boneCount; i++) {
      if (this.runtime!.getHumanAnchorBody(scene.human, i) !== 0n) anchors += 1;
    }
    this.hud[0] = scene.handles.length;
    this.hud[1] = anchors;
    return scene.handles;
  }
}

new ExtraParallelAnchorsWorker();
