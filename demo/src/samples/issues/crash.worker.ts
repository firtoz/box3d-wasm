import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyHandle, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { buildCrashDynamicBodies, buildCrashGround, crashGroundSize } from "./crash-scene";

class CrashWorker extends PhysicsWorkerBase {
  private boxes: BodyHandle[] = [];

  protected setupGround(): void {
    buildCrashGround(this.world!);
  }

  protected getGroundSize(): Vec3 {
    return crashGroundSize();
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    this.boxes = buildCrashDynamicBodies(this.world!, this.runtime!);
    return this.boxes;
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type !== "add-joint") return false;
    if (this.boxes.length < 2) return true;
    this.world!.createWeldJoint(this.boxes[0]!, this.boxes[1]!);
    return true;
  }
}

new CrashWorker();
