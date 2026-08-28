import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyId, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildExtraForcesDynamicBodies, extraForcesGroundSize, extraForcesHandleIndex } from "./forces-scene";

class ExtraForcesWorker extends PhysicsWorkerBase {
  private hover: BodyId | null = null;
  private impulse: BodyId | null = null;
  private spin: BodyId | null = null;

  protected getGroundSize(): Vec3 {
    return extraForcesGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const handles = buildExtraForcesDynamicBodies(this.world!, this.runtime!);
    this.hover = handles[extraForcesHandleIndex.hover]!;
    this.impulse = handles[extraForcesHandleIndex.impulse]!;
    this.spin = handles[extraForcesHandleIndex.spin]!;
    return handles;
  }

  protected stepPhysics(): void {
    if (this.hover !== null) {
      this.world!.applyForceToCenter(this.hover, [0, 12000, 0]);
    }
    super.stepPhysics();
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "impulse" && this.impulse !== null) {
      this.world!.applyLinearImpulseToCenter(this.impulse, [0, 8, 0]);
      return true;
    }
    if (msg.type === "spin" && this.spin !== null) {
      this.world!.applyAngularImpulse(this.spin, [0, 4, 1]);
      return true;
    }
    if (msg.type === "nudge" && this.impulse !== null) {
      this.world!.applyForce(this.impulse, [0, 0, 4000], [0.4, 0.4, 0]);
      return true;
    }
    return false;
  }
}

new ExtraForcesWorker();
