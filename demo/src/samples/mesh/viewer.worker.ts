import type { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { PhysicsWorkerCommand } from "../../physics-worker-protocol";
import { loadViewerMesh, viewerDefaultOptions, viewerGroundSize, type ViewerOptions } from "./viewer-scene";

class ViewerWorker extends PhysicsWorkerBase {
  private mesh: MeshHandle | null = null;
  private body: BodyId | null = null;
  private options: ViewerOptions = { ...viewerDefaultOptions };

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return viewerGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return this.reload();
  }

  protected handleCustomCommand(cmd: PhysicsWorkerCommand): boolean {
    const msg = cmd as Record<string, unknown>;
    if (msg.type === "set-mesh" && typeof msg.value === "number") {
      this.options.meshIndex = msg.value | 0;
      void this.reloadAndReset();
      return true;
    }
    if (msg.type === "set-median" && typeof msg.value === "boolean") {
      this.options.medianSplit = msg.value;
      void this.reloadAndReset();
      return true;
    }
    if (msg.type === "set-edges" && typeof msg.value === "boolean") {
      this.options.concaveEdges = msg.value;
      void this.reloadAndReset();
      return true;
    }
    if (msg.type === "set-weld" && typeof msg.value === "boolean") {
      this.options.weldVertices = msg.value;
      void this.reloadAndReset();
      return true;
    }
    return false;
  }

  private async reloadAndReset(): Promise<void> {
    if (this.world === null) return;
    const handles = await this.reload();
    this.setTrackedBodies(handles);
  }

  private async reload(): Promise<BodyId[]> {
    if (this.world === null) return [];
    if (this.body !== null) this.world.destroyBody(this.body);
    if (this.mesh !== null) this.world.destroyMesh(this.mesh);
    this.body = null;
    this.mesh = null;
    const loaded = await loadViewerMesh(this.world, this.options);
    this.body = loaded.body;
    this.mesh = loaded.mesh;
    return [loaded.body];
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world !== null && this.mesh !== null) this.world.destroyMesh(this.mesh);
    this.mesh = null;
    this.body = null;
  }
}

new ViewerWorker();
