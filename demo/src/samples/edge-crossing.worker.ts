import { PhysicsWorkerBase } from "../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildEdgeCrossingDynamicBodies, edgeCrossingGroundSize } from "./edge-crossing-scene";

class EdgeCrossingWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return edgeCrossingGroundSize();
  }

  protected async buildScene(): Promise<number[]> {
    return buildEdgeCrossingDynamicBodies(this.world!, this.runtime!);
  }
}

new EdgeCrossingWorker();
