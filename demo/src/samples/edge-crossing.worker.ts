import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../physics-worker-base";
import { buildEdgeCrossingDynamicBodies, edgeCrossingGroundSize } from "./edge-crossing-scene";

class EdgeCrossingWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return edgeCrossingGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildEdgeCrossingDynamicBodies(this.world!, this.runtime!);
  }
}

new EdgeCrossingWorker();
