import { PhysicsWorkerBase } from "../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class CylinderWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const hull = this.runtime!.createCylinder(1, 0.25, 0, 12);
    const body = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 2, 0], isAwake: true });
    this.runtime!.createShapeFromHull(body, hull, { density: 1000, rollingResistance: 0.05 });
    this.runtime!.destroyHull(hull);
    handles.push(body);
    return handles;
  }
}

new CylinderWorker();
