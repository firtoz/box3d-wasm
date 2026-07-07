import { PhysicsWorkerBase } from "../../physics-worker-base";
import type { Vec3 } from "box3d-wasm";
import { buildConveyorBeltDynamicBodies, conveyorBeltGroundSize } from "./conveyor-belt-scene";

class ConveyorBeltWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return conveyorBeltGroundSize(); }
  protected async buildScene(): Promise<number[]> { return buildConveyorBeltDynamicBodies(this.world!, this.runtime!); }
}

new ConveyorBeltWorker();
