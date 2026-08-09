import { BodyId, MeshHandle, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import {
  buildChainsScene,
  chainsGroundSize,
  dumpStep,
  type ChainsState,
} from "./chains-scene";

class ChainsWorker extends PhysicsWorkerBase {
  private chainsState: ChainsState | null = null;
  private mesh: MeshHandle | null = null;

  protected setupGround(): void {
    // Wave mesh ground is created in buildChainsScene.
  }

  protected getGroundSize(): Vec3 {
    return chainsGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    const { links, state } = buildChainsScene(this.world!, this.runtime!);
    this.chainsState = state;
    this.mesh = state.mesh;
    return links;
  }

  protected stepPhysics(): void {
    if (this.chainsState !== null && this.runtime !== null) {
      dumpStep(this.world!, this.runtime, [], this.totalSteps + 1, this.fixedTimeStep, this.chainsState);
    }
    super.stepPhysics();
  }

  protected onBeforeDisposeWorld(): void {
    if (this.mesh !== null && this.world !== null) {
      this.world.destroyMesh(this.mesh);
      this.mesh = null;
    }
    this.chainsState = null;
  }
}

new ChainsWorker();
