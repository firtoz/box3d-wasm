import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import {
  advanceRayCurtainOffset,
  buildRayCurtainDynamicBodies,
  castRayCurtain,
  createRayCurtainDumpState,
  RAY_CURTAIN_HEADER_FLOATS,
  RAY_CURTAIN_RAY_COUNT,
  RAY_CURTAIN_RAY_STRIDE_FLOATS,
  type RayCurtainDumpState,
  rayCurtainGroundSize,
} from "./ray-curtain-scene";

class RayCurtainWorker extends PhysicsWorkerBase {
  private rayBuffer: SharedArrayBuffer | null = null;
  private rayValues: Float32Array | null = null;
  private curtain: RayCurtainDumpState = createRayCurtainDumpState();

  protected setupGround(): void {
    // Upstream has no physics ground — only a draw grid.
  }

  protected getGroundSize(): Vec3 {
    return rayCurtainGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const floats = RAY_CURTAIN_HEADER_FLOATS + RAY_CURTAIN_RAY_COUNT * RAY_CURTAIN_RAY_STRIDE_FLOATS;
    this.rayBuffer = new SharedArrayBuffer(floats * 4);
    this.rayValues = new Float32Array(this.rayBuffer);
    const handles = buildRayCurtainDynamicBodies(this.world!, this.runtime!);
    castRayCurtain(this.world!, this.curtain.offset, this.rayValues);
    return handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.rayBuffer === null ? {} : { rays: this.rayBuffer };
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    // Mirror upstream Sample::Step then Render: cast at current offset, then advance.
    if (this.world !== null && this.rayValues !== null) {
      castRayCurtain(this.world, this.curtain.offset, this.rayValues);
    }
    advanceRayCurtainOffset(this.curtain);
  }
}

new RayCurtainWorker();
