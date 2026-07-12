import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import {
  buildJunkyardGround,
  buildJunkyardRocksAndPusher,
  junkyardGroundSize,
  stepJunkyard,
  type JunkyardState,
} from "./junkyard-scene";

class JunkyardWorker extends PhysicsWorkerBase {
  private junkyardState: JunkyardState | null = null;

  protected setupGround(): void {
    buildJunkyardGround(this.world!, this.runtime!);
  }

  protected getGroundSize(): Vec3 {
    return junkyardGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    const { rocks, pusher, state } = buildJunkyardRocksAndPusher(this.world!, this.runtime!);
    this.junkyardState = state;
    return [...rocks, pusher];
  }

  protected stepPhysics(): void {
    if (this.junkyardState !== null && this.runtime !== null) {
      stepJunkyard(this.runtime, this.junkyardState);
    }
    super.stepPhysics();
  }

  protected onBeforeDisposeWorld(): void {
    this.junkyardState = null;
  }
}

new JunkyardWorker();
