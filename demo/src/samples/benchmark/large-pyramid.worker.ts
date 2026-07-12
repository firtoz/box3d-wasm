import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildLargePyramidDynamicBodies, largePyramidGroundSize } from "./large-pyramid-scene";

class LargePyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return largePyramidGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildLargePyramidDynamicBodies(this.world!, this.runtime!); }
  protected setupGround(): void {
    const ground = this.world!.createBody({ type: 0, position: [0, -1, 0] });
    this.runtime!.createHullShape(ground, largePyramidGroundSize(), {});
  }
  protected configureScene(): void {
    this.runtime!.enableWorldContinuous(this.world!.handle, false);
  }
}

new LargePyramidWorker();
