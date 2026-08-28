import { BodyType, type BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildMotionLocksDynamicBodies, motionLocksGroundSize } from "./motion-locks-scene";

class MotionLocksWorker extends PhysicsWorkerBase {
  private bodies: BodyId[] = [];
  private locks = {
    lockX: false,
    lockY: false,
    lockLinearZ: false,
    lockRotationX: false,
    lockRotationY: false,
    lockRotationZ: false,
  };

  protected getGroundSize(): Vec3 { return motionLocksGroundSize(); }

  protected async buildScene(): Promise<BodyId[]> {
    const ground = this.world!.createBody({ type: BodyType.Static, position: [0, -1, 0] });
    this.runtime!.createHullShape(ground, motionLocksGroundSize(), {});
    const handles = [ground, ...buildMotionLocksDynamicBodies(this.world!, this.runtime!)];
    this.bodies = handles.slice(2);
    return handles;
  }

  private applyLocks(): void {
    const world = this.world;
    if (world === null) return;
    for (const body of this.bodies) {
      world.setBodyMotionLocks(body, this.locks);
      world.setBodyAwake(body, true);
    }
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    const world = this.world;
    if (world === null) return false;
    if (msg.type === "impulse") {
      const first = this.bodies[0];
      if (first !== undefined) world.applyLinearImpulseToCenter(first, [100, 0, 0], true);
      return true;
    }
    const flag = typeof msg.value === "boolean" ? msg.value : undefined;
    if (flag === undefined) return false;
    if (msg.type === "lock-linear-x") { this.locks.lockX = flag; this.applyLocks(); return true; }
    if (msg.type === "lock-linear-y") { this.locks.lockY = flag; this.applyLocks(); return true; }
    if (msg.type === "lock-linear-z") { this.locks.lockLinearZ = flag; this.applyLocks(); return true; }
    if (msg.type === "lock-angular-x") { this.locks.lockRotationX = flag; this.applyLocks(); return true; }
    if (msg.type === "lock-angular-y") { this.locks.lockRotationY = flag; this.applyLocks(); return true; }
    if (msg.type === "lock-angular-z") { this.locks.lockRotationZ = flag; this.applyLocks(); return true; }
    return false;
  }
}

new MotionLocksWorker();
