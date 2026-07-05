import { PhysicsWorkerBase } from "../../physics-worker-base";
import { BodyType, type Vec3 } from "box3d-wasm";

class BoxHullWorker extends PhysicsWorkerBase {
  protected getGroundSize(): Vec3 {
    return [10, 1, 10];
  }

  protected async buildScene(): Promise<number[]> {
    const handles: number[] = [];

    let body = this.world!.createBody({ type: BodyType.Dynamic, position: [0, 2, 0] });
    this.runtime!.createHullShape(body, [0.5, 0.25, 0.125], {});
    handles.push(body);

    body = this.world!.createBody({ type: BodyType.Dynamic, position: [2, 4, 0] });
    this.runtime!.createTransformedHullShape(body, [0.5, 0.25, 0.125], { position: [0, 0.5, 0], rotation: [0, 0, 0.382683, 0.92388] }, [1, 1, 1], {});
    handles.push(body);

    return handles;
  }
}

new BoxHullWorker();
