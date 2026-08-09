import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import {
  buildCapsuleCastRayDynamicBodies,
  CAPSULE_CAST_RAY_HEADER_FLOATS,
  CAPSULE_CAST_RAY_STRIDE_FLOATS,
  capsuleCastRayGroundSize,
  castCapsuleCastRay,
} from "./capsule-cast-ray-scene";

class CapsuleCastRayWorker extends PhysicsWorkerBase {
  private handles: number[] = [];
  private rayBuffer: SharedArrayBuffer | null = null;
  private rayValues: Float32Array | null = null;

  protected setupGround(): void {
    // Upstream draws a ground grid only; no physics ground body.
  }

  protected getGroundSize(): Vec3 {
    return capsuleCastRayGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const floats = CAPSULE_CAST_RAY_HEADER_FLOATS + CAPSULE_CAST_RAY_STRIDE_FLOATS;
    this.rayBuffer = new SharedArrayBuffer(floats * 4);
    this.rayValues = new Float32Array(this.rayBuffer);
    this.handles = buildCapsuleCastRayDynamicBodies(this.world!, this.runtime!);
    castCapsuleCastRay(this.world!, this.handles, this.rayValues);
    return this.handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.rayBuffer === null ? {} : { rays: this.rayBuffer };
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    if (this.world !== null && this.rayValues !== null) {
      castCapsuleCastRay(this.world, this.handles, this.rayValues);
    }
  }
}

new CapsuleCastRayWorker();
