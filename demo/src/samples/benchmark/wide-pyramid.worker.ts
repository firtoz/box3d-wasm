import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildWidePyramidDynamicBodies, widePyramidGroundSize } from "./wide-pyramid-scene";

class WidePyramidWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return widePyramidGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildWidePyramidDynamicBodies(this.world!, this.runtime!); }
}

new WidePyramidWorker();
