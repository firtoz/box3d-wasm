import { BodyType } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildStallDynamicBodies, stallGroundSize } from "./stall-scene";

class StallWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return stallGroundSize(); }
  protected async buildScene(): Promise<number[]> {
    const ground = this.world!.createBody({ type: BodyType.Static, position: [0, -1, 0] });
    this.runtime!.createHullShape(ground, stallGroundSize(), {});
    return [ground, ...buildStallDynamicBodies(this.world!, this.runtime!)];
  }
}

new StallWorker();
