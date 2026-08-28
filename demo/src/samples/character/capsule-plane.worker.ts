import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildCapsulePlaneDynamicBodies,
  CAPSULE_PLANE_CAPACITY,
  CAPSULE_PLANE_HEADER_FLOATS,
  CAPSULE_PLANE_STRIDE_FLOATS,
  capsulePlaneDefaultOrigin,
  capsulePlaneGroundSize,
  collectCapsulePlane,
  solveCapsulePlane,
  writeCapsulePlaneBuffer,
} from "./capsule-plane-scene";

class CapsulePlaneWorker extends PhysicsWorkerBase {
  private origin: Vec3 = [...capsulePlaneDefaultOrigin];
  private planeBuffer: SharedArrayBuffer | null = null;
  private planeValues: Float32Array | null = null;

  protected setupGround(): void {
    // Upstream draws a ground grid only; no physics ground body.
  }

  protected getGroundSize(): Vec3 {
    return capsulePlaneGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const floats = CAPSULE_PLANE_HEADER_FLOATS + CAPSULE_PLANE_CAPACITY * CAPSULE_PLANE_STRIDE_FLOATS;
    this.planeBuffer = new SharedArrayBuffer(floats * 4);
    this.planeValues = new Float32Array(this.planeBuffer);
    this.origin = [...capsulePlaneDefaultOrigin];
    const handles = buildCapsulePlaneDynamicBodies(this.world!, this.runtime!);
    this.capture();
    return handles;
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.planeBuffer === null ? {} : { planes: this.planeBuffer };
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    this.capture();
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "solve") {
      this.origin = solveCapsulePlane(this.world!, this.origin);
      this.capture();
      return true;
    }
    if (msg.type === "set-origin" && Array.isArray(msg.value) && msg.value.length === 3) {
      this.origin = [Number(msg.value[0]), Number(msg.value[1]), Number(msg.value[2])];
      this.capture();
      return true;
    }
    return false;
  }

  private capture(): void {
    if (this.world === null || this.planeValues === null) return;
    writeCapsulePlaneBuffer(this.origin, collectCapsulePlane(this.world, this.origin), this.planeValues);
  }
}

new CapsulePlaneWorker();
