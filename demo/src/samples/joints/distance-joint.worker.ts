import {type Vec3, type BodyId} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildDistanceJointDynamicBodies, distanceJointGroundSize } from "./distance-joint-scene";

class DistanceJointWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return distanceJointGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildDistanceJointDynamicBodies(this.world!, this.runtime!);
  }
}

new DistanceJointWorker();
