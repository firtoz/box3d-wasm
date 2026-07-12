import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { BodyHandle, Vec3 } from "box3d-wasm";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import {
  buildStaticInvokeDynamicBodies,
  createStaticInvokeStatic,
  staticInvokeGroundSize,
} from "./static-invoke-scene";

class StaticInvokeWorker extends PhysicsWorkerBase {
  private invoke = false;
  private staticBody: BodyHandle | null = null;
  private handles: BodyHandle[] = [];

  protected getGroundSize(): Vec3 {
    return staticInvokeGroundSize();
  }

  protected getTrackedBodyCapacity(): number {
    return 2;
  }

  protected async buildScene(): Promise<BodyHandle[]> {
    this.handles = buildStaticInvokeDynamicBodies(this.world!, this.runtime!);
    return this.handles;
  }

  protected stepPhysics(): void {
    super.stepPhysics();
    // Upstream CreateStatic at m_stepCount == 20.
    if (this.totalSteps === 20 && this.staticBody === null) {
      this.createStatic();
    }
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "set-invoke") {
      this.invoke = msg.value === true;
      return true;
    }
    if (msg.type === "create-static") {
      // Upstream CreateStatic destroys any existing static then recreates.
      this.createStatic();
      return true;
    }
    if (msg.type === "destroy-static") {
      this.destroyStatic();
      return true;
    }
    return false;
  }

  private createStatic(): void {
    this.destroyStatic();
    this.staticBody = createStaticInvokeStatic(this.world!, this.runtime!, this.invoke);
    this.handles = [this.handles[0]!, this.staticBody];
    this.setTrackedBodies(this.handles);
  }

  private destroyStatic(): void {
    if (this.staticBody === null) return;
    this.world!.destroyBody(this.staticBody);
    this.staticBody = null;
    this.handles = this.handles.slice(0, 1);
    this.setTrackedBodies(this.handles);
  }
}

new StaticInvokeWorker();
