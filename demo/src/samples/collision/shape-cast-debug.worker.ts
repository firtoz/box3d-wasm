import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { runShapeCastDebug, shapeCastDebugGroundSize } from "./shape-cast-debug-scene";

class ShapeCastDebugWorker extends PhysicsWorkerBase {
  private buffer: SharedArrayBuffer | null = null;
  private values: Float32Array | null = null;

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return shapeCastDebugGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    this.buffer = new SharedArrayBuffer(8 * 4);
    this.values = new Float32Array(this.buffer);
    this.refresh();
    return [];
  }

  protected getReadyExtra(): Record<string, unknown> {
    return this.buffer === null ? {} : { cast: this.buffer };
  }

  protected stepPhysics(): void {
    this.refresh();
  }

  private refresh(): void {
    if (this.runtime === null || this.values === null) return;
    const result = runShapeCastDebug(this.runtime);
    this.values[0] = result.h;
    this.values[1] = result.f;
    this.values[2] = result.p[0];
    this.values[3] = result.p[1];
    this.values[4] = result.p[2];
    this.values[5] = result.n[0];
    this.values[6] = result.n[1];
    this.values[7] = result.n[2];
  }
}

new ShapeCastDebugWorker();
