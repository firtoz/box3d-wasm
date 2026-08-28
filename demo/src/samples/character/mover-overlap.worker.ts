import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildMoverOverlapDynamicBodies,
  collectMoverOverlap,
  MOVER_OVERLAP_CAPACITY,
  MOVER_OVERLAP_HEADER_FLOATS,
  MOVER_OVERLAP_STRIDE_FLOATS,
  moverOverlapDefaultOrigin,
  moverOverlapGroundSize,
  solveMoverOverlap,
  writeMoverOverlapBuffer,
} from "./mover-overlap-scene";

class MoverOverlapWorker extends PhysicsWorkerBase {
  private origin: Vec3 = [...moverOverlapDefaultOrigin];
  private planeBuffer: SharedArrayBuffer | null = null;
  private planeValues: Float32Array | null = null;

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return moverOverlapGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const floats = MOVER_OVERLAP_HEADER_FLOATS + MOVER_OVERLAP_CAPACITY * MOVER_OVERLAP_STRIDE_FLOATS;
    this.planeBuffer = new SharedArrayBuffer(floats * 4);
    this.planeValues = new Float32Array(this.planeBuffer);
    this.origin = [...moverOverlapDefaultOrigin];
    const handles = buildMoverOverlapDynamicBodies(this.world!, this.runtime!);
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
    if (msg.type === "set-origin" && Array.isArray(msg.value) && msg.value.length === 3) {
      this.origin = [Number(msg.value[0]), Number(msg.value[1]), Number(msg.value[2])];
      this.capture();
      return true;
    }
    return false;
  }

  private capture(): void {
    if (this.world === null || this.planeValues === null) return;
    const planes = collectMoverOverlap(this.world, this.origin);
    const solved = solveMoverOverlap(this.world, this.origin, planes);
    writeMoverOverlapBuffer(this.origin, planes, solved, this.planeValues);
  }
}

new MoverOverlapWorker();
