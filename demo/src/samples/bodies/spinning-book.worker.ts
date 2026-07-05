import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class SpinningBookWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];
    const halfWidths: Vec3 = [0.35, 0.08, 0.5];

    let body = this.world!.createBody({
      type: BodyType.Dynamic, position: [-2, 2, 0], gravityScale: 0,
      angularVelocity: [5, 0.01, 0.01], isAwake: true,
    });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    body = this.world!.createBody({
      type: BodyType.Dynamic, position: [0, 2, 0], gravityScale: 0,
      angularVelocity: [0.01, 5, 0.01], isAwake: true,
    });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    body = this.world!.createBody({
      type: BodyType.Dynamic, position: [2, 2, 0], gravityScale: 0,
      angularVelocity: [0.01, 0.01, -5], isAwake: true,
    });
    this.runtime!.createHullShape(body, halfWidths, {});
    handles.push(body);

    return handles;
  }
}

new SpinningBookWorker();
