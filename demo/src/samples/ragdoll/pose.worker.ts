import type { BodyId, HumanHandle, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildPoseScene, poseGroundSize } from "./pose-scene";

class PoseWorker extends PhysicsWorkerBase {
  private human: HumanHandle | null = null;
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return poseGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const built = buildPoseScene(this.world!, this.runtime!);
    this.human = built.human;
    this.mesh = built.mesh;
    return built.handles;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world === null) return;
    if (this.human !== null) this.world.destroyHuman(this.human);
    if (this.mesh !== null) this.world.destroyMesh(this.mesh);
    this.human = null;
    this.mesh = null;
  }
}

new PoseWorker();
