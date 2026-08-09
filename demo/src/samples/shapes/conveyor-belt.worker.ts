import { BodyId, Vec3 } from "box3d-wasm";
import { PhysicsWorkerBase } from "../../physics-worker-base";
import { buildConveyorBeltDynamicBodies, conveyorBeltGroundSize } from "./conveyor-belt-scene";

class ConveyorBeltWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 { return conveyorBeltGroundSize(); }
  protected async buildScene(): Promise<BodyId[]> { return buildConveyorBeltDynamicBodies(this.world!, this.runtime!); }
}

new ConveyorBeltWorker();
