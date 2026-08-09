import {type Vec3, type BodyId} from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildParallelSpringDynamicBodies, parallelSpringGroundSize } from "./parallel-spring-scene";

class ParallelSpringWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return parallelSpringGroundSize();
  }

  protected async buildScene(): Promise<BodyId[]> {
    return buildParallelSpringDynamicBodies(this.world!, this.runtime!);
  }
}

new ParallelSpringWorker();
