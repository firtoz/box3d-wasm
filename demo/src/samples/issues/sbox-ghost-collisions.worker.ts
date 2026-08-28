import type { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildSboxGhostDynamicBodies,
  dumpStep,
  sboxGhostGroundSize,
  type GhostWalkState,
} from "./sbox-ghost-collisions-scene";

class SboxGhostWorker extends PhysicsWorkerBase {
  private meshes: MeshHandle[] = [];
  private walk: GhostWalkState = { dirX: 1, dirZ: 1 };
  private handles: BodyId[] = [];

  protected setupGround(): void {}

  protected getGroundSize(): Vec3 {
    return sboxGhostGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const built = buildSboxGhostDynamicBodies(this.world!, this.runtime!);
    this.meshes = built.meshes;
    this.handles = built.handles;
    this.walk = { dirX: 1, dirZ: 1 };
    return built.handles;
  }

  protected stepPhysics(): void {
    if (this.world === null || this.runtime === null) return;
    dumpStep(this.world, this.runtime, this.handles, this.totalSteps, this.fixedTimeStep, this.walk);
    this.world.step(this.fixedTimeStep, this.subSteps);
    this.totalSteps += 1;
  }

  protected onBeforeDisposeWorld(): void {
    if (this.world === null) return;
    for (let i = this.meshes.length - 1; i >= 0; i--) this.world.destroyMesh(this.meshes[i]!);
    this.meshes = [];
  }
}

new SboxGhostWorker();
