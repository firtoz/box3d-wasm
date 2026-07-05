import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class GyroscopicTorqueWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const body = this.world!.createBody({
      type: BodyType.Dynamic,
      position: [0, 2, 0],
      gravityScale: 0,
    });

    const cylinder = this.runtime!.createCylinder(0.6, 0.15, 0, 32);

    this.runtime!.createShapeFromHull(body, cylinder, {
      density: 1,
      updateBodyMass: false,
    });

    this.runtime!.createHullShape(body, [1, 0.05, 0.1], {
      density: 1,
      updateBodyMass: false,
    });

    this.runtime!.applyBodyMassFromShapes(body);

    this.runtime!.setBodyAngularVelocity(body, [0.01, 0.01, 10]);

    this.runtime!.destroyHull(cylinder);

    return [body];
  }
}

new GyroscopicTorqueWorker();
